# Building OptiScaler DLL

This branch includes a Windows build script for local Visual Studio builds.

## Requirements

- Windows 10/11
- Git
- Visual Studio 2022 or Visual Studio Build Tools 2022
- Visual Studio workload: **Desktop development with C++**
- MSVC v143 toolset
- Windows 10 SDK

## Fresh checkout

```powershell
git clone --recursive -b fsr-sdk-22-rr-wukong https://github.com/phantoslayer21/OptiScaler.git
cd OptiScaler
```

If the repo was already cloned without submodules:

```powershell
git submodule update --init --recursive
```

## Build Release x64

From the repo root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-optiscaler.ps1 -Configuration Release -Platform x64 -Clean
```

Or use the batch wrapper:

```cmd
build-optiscaler.bat -Configuration Release -Platform x64 -Clean
```

## Build ReleaseDebug x64

Use this when you want release-like performance with debug symbols/logging:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-optiscaler.ps1 -Configuration ReleaseDebug -Platform x64 -Clean
```

## Output

The Visual Studio project already stages the Release package under:

```text
x64\Release\a\
```

The build script also copies the resulting package to:

```text
artifacts\OptiScaler-Release-x64\
```

For `ReleaseDebug`, the equivalent artifact folder is:

```text
artifacts\OptiScaler-ReleaseDebug-x64\
```

## What the script does

- Finds the repo root by looking for `OptiScaler.sln`.
- Updates submodules unless `-SkipSubmodules` is passed.
- Locates MSBuild through `vswhere.exe`, then known Visual Studio 2022 install paths.
- Builds `OptiScaler.sln` with the selected configuration/platform.
- Fails if `OptiScaler.dll` is not found in the expected output paths.
- Copies staged output into `artifacts\` unless `-NoPackageCopy` is passed.

## Useful flags

```powershell
-SkipSubmodules   # Do not run git submodule sync/update
-NoPackageCopy    # Build only; do not copy to artifacts
-MSBuildPath      # Explicit MSBuild.exe path
-RepoRoot         # Explicit repo root if running script from another folder
```

## If the build fails

Run the script again with `ReleaseDebug` first. If it still fails, copy the first compiler or linker error, not the final summary line. The first real error is the useful one.

Common setup issues:

- Missing Visual Studio C++ workload.
- Submodules not initialized.
- Missing Windows SDK.
- Antivirus/Defender locking generated DLLs during post-build copy.
- 7-Zip not installed for `ReleaseDebug` post-build archive steps.
