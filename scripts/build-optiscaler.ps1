<#
.SYNOPSIS
    Builds the OptiScaler DLL with Visual Studio 2022 / MSBuild.

.DESCRIPTION
    Run this from anywhere inside the repository or pass -RepoRoot explicitly.
    The script restores/updates submodules, locates MSBuild, builds OptiScaler.sln,
    and copies the staged Release package to artifacts\OptiScaler-<config>-<platform>.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File .\scripts\build-optiscaler.ps1

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File .\scripts\build-optiscaler.ps1 -Configuration ReleaseDebug -Clean
#>

[CmdletBinding()]
param(
    [ValidateSet('Release', 'ReleaseDebug', 'Debug')]
    [string]$Configuration = 'Release',

    [ValidateSet('x64', 'Win32')]
    [string]$Platform = 'x64',

    [string]$RepoRoot,

    [switch]$Clean,
    [switch]$SkipSubmodules,
    [switch]$NoPackageCopy,
    [string]$MSBuildPath
)

$ErrorActionPreference = 'Stop'

function Write-Step {
    param([string]$Message)
    Write-Host "`n==> $Message" -ForegroundColor Cyan
}

function Resolve-RepoRoot {
    param([string]$ExplicitRoot)

    if ($ExplicitRoot) {
        $root = Resolve-Path -LiteralPath $ExplicitRoot
        return $root.Path
    }

    $candidate = Get-Location
    while ($candidate) {
        if (Test-Path -LiteralPath (Join-Path $candidate.Path 'OptiScaler.sln')) {
            return $candidate.Path
        }
        $candidate = $candidate.Parent
    }

    throw 'Could not find OptiScaler.sln. Run this script inside the repo or pass -RepoRoot.'
}

function Find-MSBuild {
    param([string]$ExplicitPath)

    if ($ExplicitPath) {
        if (Test-Path -LiteralPath $ExplicitPath) {
            return (Resolve-Path -LiteralPath $ExplicitPath).Path
        }
        throw "MSBuildPath was provided but does not exist: $ExplicitPath"
    }

    $programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
    $programFiles = [Environment]::GetFolderPath('ProgramFiles')

    $vswhere = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        $path = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find 'MSBuild\Current\Bin\MSBuild.exe' | Select-Object -First 1
        if ($path -and (Test-Path -LiteralPath $path)) {
            return $path
        }
    }

    $fallbackRelativePaths = @(
        'Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe',
        'Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe',
        'Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe',
        'Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe'
    )

    foreach ($relativePath in $fallbackRelativePaths) {
        $path = Join-Path $programFiles $relativePath
        if (Test-Path -LiteralPath $path) {
            return $path
        }
    }

    throw 'Could not find MSBuild. Install Visual Studio 2022 with Desktop development with C++, or pass -MSBuildPath.'
}

function Invoke-Checked {
    param(
        [string]$FilePath,
        [string[]]$Arguments,
        [string]$FailureMessage
    )

    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$FailureMessage Exit code: $LASTEXITCODE"
    }
}

$repo = Resolve-RepoRoot -ExplicitRoot $RepoRoot
$solution = Join-Path $repo 'OptiScaler.sln'
$artifactRoot = Join-Path $repo 'artifacts'
$artifactDir = Join-Path $artifactRoot "OptiScaler-$Configuration-$Platform"

Write-Step "Repository: $repo"
Set-Location -LiteralPath $repo

if (-not $SkipSubmodules) {
    Write-Step 'Updating git submodules'
    Invoke-Checked -FilePath 'git' -Arguments @('submodule', 'sync', '--recursive') -FailureMessage 'git submodule sync failed.'
    Invoke-Checked -FilePath 'git' -Arguments @('submodule', 'update', '--init', '--recursive') -FailureMessage 'git submodule update failed.'
}

$msbuild = Find-MSBuild -ExplicitPath $MSBuildPath
Write-Step "Using MSBuild: $msbuild"

if ($Clean) {
    Write-Step "Cleaning $Configuration|$Platform"
    Invoke-Checked -FilePath $msbuild -Arguments @(
        $solution,
        '/t:Clean',
        "/p:Configuration=$Configuration",
        "/p:Platform=$Platform",
        '/m'
    ) -FailureMessage 'MSBuild clean failed.'
}

Write-Step "Building $Configuration|$Platform"
Invoke-Checked -FilePath $msbuild -Arguments @(
    $solution,
    '/t:Build',
    "/p:Configuration=$Configuration",
    "/p:Platform=$Platform",
    '/m',
    '/v:minimal',
    '/p:UseMultiToolTask=true',
    '/p:EnforceProcessCountAcrossBuilds=true'
) -FailureMessage 'MSBuild build failed.'

$platformOut = if ($Platform -eq 'x64') { 'x64' } else { 'Win32' }
$outDir = Join-Path $repo "$platformOut\$Configuration"
$stagedDir = Join-Path $outDir 'a'
$dllCandidates = @(
    (Join-Path $stagedDir 'OptiScaler.dll'),
    (Join-Path $outDir 'OptiScaler.dll')
)

$dll = $dllCandidates | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
if (-not $dll) {
    throw "Build completed, but OptiScaler.dll was not found in expected locations: $($dllCandidates -join ', ')"
}

Write-Step "Built DLL: $dll"

if (-not $NoPackageCopy) {
    Write-Step "Copying build output to $artifactDir"
    New-Item -ItemType Directory -Force -Path $artifactDir | Out-Null

    if (Test-Path -LiteralPath $stagedDir) {
        Copy-Item -Path (Join-Path $stagedDir '*') -Destination $artifactDir -Recurse -Force
    } else {
        Copy-Item -LiteralPath $dll -Destination $artifactDir -Force
    }

    Write-Host "Artifact folder: $artifactDir" -ForegroundColor Green
}

Write-Host "`nBuild succeeded." -ForegroundColor Green
Write-Host "DLL: $dll" -ForegroundColor Green
