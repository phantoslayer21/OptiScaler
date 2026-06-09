# FSR SDK 2.2 / Ray Regeneration / Black Myth: Wukong Notes

This document is current for the `fsr-sdk-22-rr-wukong` branch.

## Current branch status

This branch is a staging branch for FSR SDK 2.2-era FidelityFX work, FSR Ray Regeneration cleanup, and Black Myth: Wukong validation. It is **not** claiming production-ready Black Myth: Wukong FSR Ray Regeneration support yet.

The branch is build-script-ready for local Windows/MSBuild testing. It has not been compiled in this ChatGPT environment because MSBuild and the Windows SDK are not available here. Use [`build-optiscaler.md`](build-optiscaler.md) for the current build path.

## Goals

- Keep the DX12 FSR upscaler path aligned with FSR 4.1-era SDK expectations.
- Keep the FSR frame generation path compatible with the split FidelityFX DLL loading model already used by OptiScaler.
- Harden FSR Ray Regeneration 1.1 configuration and dispatch behavior.
- Improve the DLSS Ray Reconstruction to FSR Ray Regeneration translation layer.
- Track Black Myth: Wukong as a dedicated validation target instead of treating it as a generic Unreal Engine title.
- Add an RR Input Doctor path so users can see which DLSS-RR inputs exist, which conversion fallbacks are active, and which assumptions are likely hurting quality.

## Implemented in this branch

- Fixed denoiser configuration key/index mapping in `FSRDFeature_Dx12.h` so local array indices are not confused with FidelityFX denoiser enum values.
- Hardened `FSRDFeatureDx12` member initialization so the first frame starts with deterministic denoiser state and identity camera matrices instead of relying on uninitialized matrix/handedness data.
- Added compile-time guards so future FidelityFX SDK denoiser key changes fail loudly instead of silently desynchronizing the config array.
- Added a Windows build script: `scripts/build-optiscaler.ps1`.
- Added a batch wrapper: `build-optiscaler.bat`.
- Added current build docs: [`docs/build-optiscaler.md`](build-optiscaler.md).
- Added an RR Input Doctor design document: [`docs/fsr-rr-input-doctor.md`](fsr-rr-input-doctor.md).
- Added a structured FSR-RR game report issue template: [`.github/ISSUE_TEMPLATE/fsr-ray-regeneration-game-report.yml`](../.github/ISSUE_TEMPLATE/fsr-ray-regeneration-game-report.yml).
- Added a documentation status index: [`docs/documentation-status.md`](documentation-status.md).
- Updated the README to reflect branch status and build instructions.

## Important limits

The following work is still pending because the affected files are large and should be edited/tested from a local checkout rather than through lossy partial-file connector edits:

1. Update `FFX_UPSCALER_VERSION_MAJOR/MINOR/PATCH` in `OptiScaler/upscalers/fsr31/FSR31Feature_Dx12.cpp` from `4.0.3` to the SDK 2.2 / FSR 4.1 target.
2. Add a dedicated Black Myth: Wukong RR quirk in `OptiScaler/misc/Quirks.h` rather than relying only on the existing UE `b1` executable entry.
3. Use that quirk in `FSRDFeature_Dx12.cpp` to select safer defaults for:
   - roughness packed/unpacked handling,
   - matrix source priority,
   - motion-vector scale interpretation,
   - normal/depth conversion diagnostics.
4. Add runtime logging around RR conversion inputs:
   - render and output size,
   - motion-vector scale before/after UV conversion,
   - depth mode,
   - roughness source,
   - matrix source.
5. Implement the RR Input Doctor state collector and overlay/log output described in [`fsr-rr-input-doctor.md`](fsr-rr-input-doctor.md).

## Missing feature: RR Input Doctor

The highest-value usability/quality feature still missing from the code is an **FSR-RR Input Doctor**. Ray regeneration failures are usually not self-explanatory: a bad result can come from missing hit distance, roughness packing, bad motion-vector scale, wrong handedness, invalid matrices, depth mode mismatch, bad reset/history state, or a denoiser tuning problem.

The Input Doctor should expose those facts directly in the overlay and logs. The intended design is documented in [`docs/fsr-rr-input-doctor.md`](fsr-rr-input-doctor.md).

## Black Myth: Wukong validation checklist

Use Black Myth: Wukong only in an offline/single-player context. Validate with the game at native DLSS Ray Reconstruction settings first, then with OptiScaler's FSR Ray Regeneration replacement path.

Capture or log:

- executable name, expected to be a `b1-*-shipping.exe` Unreal Engine binary,
- whether DLSS Ray Reconstruction exposes all expected NGX parameters,
- color buffer format and resolution,
- motion-vector texture format, sign, and scale,
- depth mode: hardware, linear, reversed-Z,
- normal buffer encoding and handedness,
- roughness source: packed in normals vs separate `GBuffer_Roughness`,
- diffuse/specular albedo availability,
- specular hit-distance availability,
- matrix source used: NGX matrices vs Streamline constants.

Visual checks:

- no explosive firefly shimmer in reflections,
- no large disocclusion smearing behind moving objects,
- no obvious ghost trails on foliage or hair,
- no roughness inversion where matte surfaces become mirror-like,
- no overdarkened indirect specular,
- stable output when panning quickly,
- stable output after resolution changes or DLSS quality-mode changes.

## Issue reports

Use the FSR-RR issue template for game reports. It captures game version, executable, GPU/driver, RR mode, available DLSS-RR inputs, symptoms, reproduction steps, and logs/captures.