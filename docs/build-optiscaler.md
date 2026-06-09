# Building OptiScaler DLL

This document is current for the `fsr-sdk-22-rr-wukong-mainline` branch, which is based on current `optiscaler/OptiScaler:master`.

## Build status

The branch includes a Windows build script and batch wrapper. The build path is ready for a local Visual Studio/MSBuild run, but it has not been compiled inside this ChatGPT environment because this sandbox does not provide Visual Studio, MSBuild, or the Windows SDK.

Treat a successful local MSBuild run as the source of truth. If it fails, fix the first compiler or linker error before chasing later summary errors.

## Requirements

- Windows 10/11
- Git
- Visual Studio 2022 or Visual Studio Build Tools 2022
- Visual Studio workload: **Desktop development with C++**
- MSVC v143 toolset
- Windows 10 SDK
- Optional: 7-Zip, only if a selected project configuration invokes `7z.exe` in post-build steps

## Fresh checkout

```powershell
git clone --recursive -b fsr-sdk-22-rr-wukong-mainline https://github.com/phantoslayer21/OptiScaler.git
cd OptiScaler
```

If the repo was already cloned without submodules:

```powershell
git submodule sync --recursive
git submodule update --init --recursive
```

## Build Release x64

Use `Release|x64` first. It is the normal DLL/package build path.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-optiscaler.ps1 -Configuration Release -Platform x64 -Clean
```

Or use the batch wrapper:

```cmd
build-optiscaler.bat -Configuration Release -Platform x64 -Clean
```

## Output

The build script looks for `OptiScaler.dll` in the normal Visual Studio output/staging folders and copies the staged package to:

```text
artifacts\OptiScaler-Release-x64\
```

## Useful flags

```powershell
-SkipSubmodules   # Do not run git submodule sync/update
-NoPackageCopy    # Build only; do not copy to artifacts
-MSBuildPath      # Explicit MSBuild.exe path
-RepoRoot         # Explicit repo root if running script from another folder
```

## If the build fails

Use the first real compiler or linker error, not the final MSBuild summary line. Common setup issues are:

- Missing Visual Studio C++ workload.
- Missing MSVC v143 toolset.
- Missing Windows SDK.
- Submodules not initialized.
- Antivirus/Defender locking generated DLLs during post-build copy.
