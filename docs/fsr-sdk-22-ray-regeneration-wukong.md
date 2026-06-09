# FSR SDK 2.2 / Ray Regeneration / Black Myth: Wukong Notes

This branch is intended to track the FSR SDK 2.2-era integration path for OptiScaler's DX12 FidelityFX stack, with special attention on FSR Ray Regeneration and DLSS Ray Reconstruction input translation.

## Goals

- Keep the DX12 FSR upscaler path aligned with FSR 4.1-era SDK expectations.
- Keep the FSR frame generation path compatible with the split FidelityFX DLL loading model already used by OptiScaler.
- Harden FSR Ray Regeneration 1.1 configuration and dispatch behavior.
- Improve the DLSS Ray Reconstruction to FSR Ray Regeneration translation layer.
- Track Black Myth: Wukong as a dedicated validation target instead of treating it as a generic Unreal Engine title.

## Implemented in this branch

- Fixed denoiser configuration key/index mapping in `FSRDFeature_Dx12.h` so local array indices are not confused with FidelityFX denoiser enum values.
- Hardened `FSRDFeatureDx12` member initialization so the first frame starts with deterministic denoiser state and identity camera matrices instead of relying on uninitialized matrix/handedness data.
- Documented branch status in the README.

## Remaining code work

The following changes should be made after full-file patching or local build access is available:

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

## Notes

This branch is not claiming production-ready Black Myth: Wukong FSR Ray Regeneration support yet. It is a safer staging branch for the SDK/RR cleanup and game-specific validation work needed to get there.
