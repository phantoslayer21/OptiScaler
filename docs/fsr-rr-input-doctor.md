# FSR Ray Regeneration Input Doctor

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

Use a simple traffic-light model:

- **OK**: expected input exists and basic dimensions/format look reasonable.
- **Fallback**: input is missing, but the converter has a known fallback path.
- **Suspicious**: input exists, but scale, dimensions, or state look inconsistent.
- **Missing**: required input is absent and RR quality or dispatch is expected to fail.

## Quality-risk flags

The doctor should also compute and display risk flags:

- `RR_RISK_NO_SPEC_HIT_DISTANCE`: mode-2 may lose reflection-space tracking.
- `RR_RISK_PACKED_ROUGHNESS_ASSUMED`: roughness was not explicit and packed-normal roughness was assumed.
- `RR_RISK_STREAMLINE_MATRIX_FALLBACK`: NGX matrices were missing or rejected.
- `RR_RISK_MV_SCALE_UNKNOWN`: motion vector scale was not provided by NGX.
- `RR_RISK_LOWRES_DEPTH_MISSING`: depth is missing when low-res motion vectors need it.
- `RR_RISK_FIRST_FRAME_RESET`: first frame or reset path is active; history should be ignored.
- `RR_RISK_DRS_REINIT`: render size changed and denoiser context was recreated.

These flags should be cheap to compute and should be emitted to both the overlay and log.

## Recommended implementation points

### `FSRDFeatureDx12::PrepareDenoiseConvInput`

This is the best place to collect input availability and source information. It already gathers the DLSS-RR buffers and decides whether NGX matrices or Streamline constants are used.

Recommended additions:

```cpp
struct FSRRInputDoctorState
{
    bool hasColor = false;
    bool hasMotionVectors = false;
    bool hasDepth = false;
    bool hasNormals = false;
    bool hasRoughness = false;
    bool assumedPackedRoughness = false;
    bool hasDiffuseAlbedo = false;
    bool hasSpecularAlbedo = false;
    bool hasSpecularHitDistance = false;
    bool usedNgxViewMatrix = false;
    bool usedStreamlineViewMatrix = false;
    bool usedNgxProjectionMatrix = false;
    bool usedStreamlineProjectionMatrix = false;
    bool motionVectorScaleKnown = false;
    bool lowResMotionVectors = false;
    bool reset = false;
};
```

Store the most recent state in `State` or directly inside `FSRDFeatureDx12`.

### `FSRDFeatureDx12::PrepareDenoiserInput`

This is the best place to record frame-specific values:

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

### `FSRDFeatureDx12::ConvertDenoiserBuffers`

This is the best place to report conversion assumptions:

- packed roughness enabled/disabled,
- hardware depth vs linear depth,
- inverse projection validity,
- previous-view matrix validity,
- debug conversion mode currently selected.

## Suggested overlay layout

```text
FSR-RR Input Doctor
Backend: FSR Ray Regeneration
Denoiser Mode: Mode 2
Denoiser Version: <version string>
Upscaler Version: <version string>

Required inputs:
[OK] Color               NGX     1920x1080  R16G16B16A16_FLOAT
[OK] Motion vectors      NGX     1920x1080  R16G16_FLOAT       scale=(-1920,1080) -> UV=(-1,1)
[OK] Depth               NGX     1920x1080  D32_FLOAT          hardware/reversed-Z
[OK] Normals             DLSSD   1920x1080  R10G10B10A2_UNORM
[Fallback] Roughness     Packed  normals.a  assumed packed
[OK] Diffuse albedo      DLSSD   1920x1080  R8G8B8A8_UNORM
[OK] Specular albedo     DLSSD   1920x1080  R8G8B8A8_UNORM
[Warn] Spec hit distance Missing optional mode-2 input

Matrices:
View: NGX
Projection: Streamline reconstructed
Near/Far: 0.1 / 100000
FOV Y: 70.0 deg
Handedness: RH -> converted LH

Risk flags:
RR_RISK_PACKED_ROUGHNESS_ASSUMED
RR_RISK_STREAMLINE_MATRIX_FALLBACK
RR_RISK_NO_SPEC_HIT_DISTANCE
```

## Why this improves quality

This does not directly change the denoiser output, but it makes every quality fix faster. Instead of guessing whether Wukong has wrong roughness, wrong motion vectors, missing hit distance, or bad matrices, the overlay/log can make that visible in one frame. That is exactly the sort of feature needed before adding per-game RR profiles.

## Recommended follow-up feature

Once the Input Doctor exists, add **Auto Profile Suggestions**:

- If roughness is missing but normals exist, suggest `Packed roughness`.
- If NGX matrices are missing but Streamline constants exist, suggest `Prefer Streamline matrices`.
- If motion vector scale is missing, suggest common UE pixel-space defaults.
- If specular hit distance is missing in mode-2, suggest trying mode-1 or lower stability bias.
- If DRS reinitializes constantly, suggest fixed render scale for RR validation.
