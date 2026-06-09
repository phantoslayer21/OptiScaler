# Documentation Status

This document is current for the `fsr-sdk-22-rr-wukong` branch.

## Authoritative branch docs

Use these docs for the current branch state:

- [`../README.md`](../README.md): branch overview, safety warning, supported-feature summary, and build entry point.
- [`build-optiscaler.md`](build-optiscaler.md): current Windows/MSBuild build instructions.
- [`fsr-sdk-22-ray-regeneration-wukong.md`](fsr-sdk-22-ray-regeneration-wukong.md): FSR SDK 2.2 / FSR-RR / Black Myth: Wukong branch status.
- [`fsr-rr-input-doctor.md`](fsr-rr-input-doctor.md): design-only proposal for the RR Input Doctor feature.
- [`.github/ISSUE_TEMPLATE/fsr-ray-regeneration-game-report.yml`](../.github/ISSUE_TEMPLATE/fsr-ray-regeneration-game-report.yml): current template for FSR-RR game reports.

## Current implementation status

Implemented in this branch:

- FSR-RR denoiser key/index mapping hardening.
- Deterministic initialization of FSR-RR denoiser context state and camera matrices.
- Compile-time guard for FidelityFX denoiser key-count drift.
- Windows build script: `scripts/build-optiscaler.ps1`.
- Batch build wrapper: `build-optiscaler.bat`.
- FSR-RR game report issue template.
- RR Input Doctor design documentation.

Not implemented yet:

- Full FSR upscaler API target bump inside `FSR31Feature_Dx12.cpp`.
- Dedicated Black Myth: Wukong RR quirk wiring inside `Quirks.h` / `FSRDFeature_Dx12.cpp`.
- RR Input Doctor overlay/log implementation.
- Runtime RR input logging.
- Verified local MSBuild result from this environment.

## Legacy docs

Some upstream-root docs such as `Config.md`, `Spoofing.md`, and feature screenshots are inherited from upstream and may describe stable OptiScaler behavior rather than the experimental FSR-RR branch work. Branch-specific changes and caveats are tracked in the docs listed above.

## Build verification status

The branch is configured for local Windows builds through `scripts/build-optiscaler.ps1`, but no Visual Studio/MSBuild run has been performed from this ChatGPT environment. Treat a successful local `Release|x64` build as the source of truth.
