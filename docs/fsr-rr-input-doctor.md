# FSR Ray Regeneration Input Doctor

This document is current for the `fsr-sdk-22-rr-wukong-mainline` branch.

## Current status

The FSR-RR Input Doctor is a **design proposal**, not an implemented overlay page yet. This mainline-based branch carries the diagnostic design forward without blindly porting the older, non-mainline RR implementation.

## Why this feature should exist

FSR Ray Regeneration is much more sensitive than normal temporal upscaling because it depends on several high-value inputs being correct at the same time: color, motion vectors, depth, normals, roughness, diffuse/specular albedo, camera matrices, frame timing, jitter, and optionally specular hit distance.

When any one of those is wrong, users usually describe the result as blur, shimmer, trails, fireflies, roughness inversion, smeared reflections, or broken disocclusion. Without a diagnostic layer, it is hard to know whether the problem is the denoiser, the upscaler, the game, an NGX parameter mismatch, a Streamline constant mismatch, or the DLSS-RR to FSR-RR conversion shader.

The missing usability feature is an **RR Input Doctor**: a single diagnostics view that tells the user and developer what OptiScaler actually received, what it assumed, what it converted, and what quality-risk flags are active.

## Intended user-facing behavior

Add an overlay page or log section called `FSR-RR Input Doctor`.

It should show one row per required input:

| Input | Status | Source | Resolution | Format | Notes |
| --- | --- | --- | --- | --- | --- |
| Color | OK / Missing | NGX | WxH | DXGI_FORMAT_* | HDR/non-HDR, gamma expectation |
| Motion vectors | OK / Missing / Suspicious | NGX | WxH | DXGI_FORMAT_* | pixel-space vs UV-space, scale X/Y |
| Depth | OK / Missing / Suspicious | NGX | WxH | DXGI_FORMAT_* | hardware depth, linear depth, reversed-Z |
| Normals | OK / Missing / Suspicious | DLSSD GBuffer | WxH | DXGI_FORMAT_* | handedness/Y flip risk |
| Roughness | OK / Packed fallback / Missing | DLSSD GBuffer | WxH | DXGI_FORMAT_* | packed channel vs standalone texture |
| Diffuse albedo | OK / Missing | DLSSD | WxH | DXGI_FORMAT_* | non-gamma flag compatibility |
| Specular albedo | OK / Missing | DLSSD | WxH | DXGI_FORMAT_* | emissive/specular clamp risk |
| Specular hit distance | OK / Optional missing | DLSSD | WxH | DXGI_FORMAT_* | mode-2 quality risk if missing |
| View matrix | OK / Streamline fallback / Missing | NGX or SL | n/a | n/a | matrix source |
| Projection matrix | OK / Reconstructed / Missing | NGX or SL | n/a | n/a | FOV/near/far source |

## Status levels

- **OK**: expected input exists and basic dimensions/format look reasonable.
- **Fallback**: input is missing, but the converter has a known fallback path.
- **Suspicious**: input exists, but scale, dimensions, or state look inconsistent.
- **Missing**: required input is absent and RR quality or dispatch is expected to fail.

## Quality-risk flags

- `RR_RISK_NO_SPEC_HIT_DISTANCE`: mode-2 may lose reflection-space tracking.
- `RR_RISK_PACKED_ROUGHNESS_ASSUMED`: roughness was not explicit and packed-normal roughness was assumed.
- `RR_RISK_STREAMLINE_MATRIX_FALLBACK`: NGX matrices were missing or rejected.
- `RR_RISK_MV_SCALE_UNKNOWN`: motion vector scale was not provided by NGX.
- `RR_RISK_LOWRES_DEPTH_MISSING`: depth is missing when low-res motion vectors need it.
- `RR_RISK_FIRST_FRAME_RESET`: first frame or reset path is active; history should be ignored.
- `RR_RISK_DRS_REINIT`: render size changed and denoiser context was recreated.

These flags should be cheap to compute and should be emitted to both the overlay and log.

## Recommended implementation points

### `PrepareDenoiseConvInput`

Collect input availability and source information where the DLSS-RR buffers are gathered and where NGX matrices or Streamline constants are selected.

### `PrepareDenoiserInput`

Record frame-specific values:

- render size,
- target/output size,
- frame index,
- delta time,
- jitter offsets,
- motion vector scale before conversion,
- motion vector scale after UV conversion,
- camera near/far,
- vertical FOV,
- camera handedness.

### `ConvertDenoiserBuffers`

Report conversion assumptions:

- packed roughness enabled/disabled,
- hardware depth vs linear depth,
- inverse projection validity,
- previous-view matrix validity,
- debug conversion mode currently selected.

## Recommended follow-up feature

Once the Input Doctor exists, add **Auto Profile Suggestions**:

- If roughness is missing but normals exist, suggest `Packed roughness`.
- If NGX matrices are missing but Streamline constants exist, suggest `Prefer Streamline matrices`.
- If motion vector scale is missing, suggest common UE pixel-space defaults.
- If specular hit distance is missing in mode-2, suggest trying mode-1 or lower stability bias.
- If DRS reinitializes constantly, suggest fixed render scale for RR validation.

## Related docs

- [`mainline-sync-status.md`](mainline-sync-status.md)
- [`build-optiscaler.md`](build-optiscaler.md)
