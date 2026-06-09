# Mainline Sync Status

This document is current for the `fsr-sdk-22-rr-wukong-mainline` branch.

## Base

This branch was created from current mainline:

```text
optiscaler/OptiScaler:master
238ca06aeb2b104f38d4869ee2977f8d35188964
```

It is intended to replace the older `fsr-sdk-22-rr-wukong` branch for any work that should be aligned with the official mainline OptiScaler repository.

## What was carried over safely

The following additive/non-invasive pieces were carried over:

- Windows build script: `scripts/build-optiscaler.ps1`
- Batch build wrapper: `build-optiscaler.bat`
- Build documentation: `docs/build-optiscaler.md`
- Mainline sync/status documentation: this file
- FSR-RR Input Doctor design document: `docs/fsr-rr-input-doctor.md`
- FSR-RR game report issue template: `.github/ISSUE_TEMPLATE/fsr-ray-regeneration-game-report.yml`

## What was not blindly carried over

The previous branch was based on `ZachHembree/OptiScaler`, not official mainline `optiscaler/OptiScaler`. It diverged from current mainline by hundreds of commits.

Because of that, the large RR code path from the previous branch was **not** blindly replayed onto mainline through the GitHub connector. The affected files include core input handling, feature providers, FSR3/FSR4 paths, shader preprocessing, proxy code, quirks, and menu code. Those should be ported from a local checkout with a real merge/rebase/build cycle.

Not safely ported yet:

- FSR Ray Regeneration implementation files from the older branch.
- DLSS Ray Reconstruction to FSR Ray Regeneration conversion shader path.
- Black Myth: Wukong-specific RR quirk wiring.
- RR runtime input logging.
- RR Input Doctor overlay/log implementation.

## Recommended next local workflow

```powershell
git clone --recursive https://github.com/phantoslayer21/OptiScaler.git
cd OptiScaler
git checkout fsr-sdk-22-rr-wukong-mainline
git remote add upstream https://github.com/optiscaler/OptiScaler.git
git fetch upstream
```

Then port the RR code from the older branch in smaller, buildable chunks:

```powershell
git checkout -b rr-port-step-1
git cherry-pick <small-commit-or-manual-patch>
.\build-optiscaler.bat -Configuration Release -Platform x64 -Clean
```

Keep each port step focused and build-verified before moving to the next one.

## Build verification status

This branch is configured for local Windows builds through `scripts/build-optiscaler.ps1`, but no Visual Studio/MSBuild run has been performed from this ChatGPT environment. Treat a successful local `Release|x64` build as the source of truth.
