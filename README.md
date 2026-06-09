<div align="center">

  ![Logo](https://github.com/user-attachments/assets/c7dad5da-0b29-4710-8a57-b58e4e407abd)

</div>
<hr />
<br />
<div align="center">
  <a href="https://github.com/sponsors/cdozdil?frequency=one-time"><img src="images/gh-sponsor-red.png" /></a>
  <a href="https://buymeacoffee.com/nitec"><img src="images/bmac.png" /></a>
</div>
<br />

## Table of Contents

**1.** [About](#about)  
**2.** [Current branch status](#current-branch-status)  
**3.** [How it works](#how-it-works)  
**4.** [Supported APIs and Upscalers](#which-apis-and-upscalers-are-supported)  
**5.** [Installation](#installation)  
**6.** [Configuration](#configuration)  
**7.** [Build](#build)  
**8.** [Branch documentation](#branch-documentation)  
**9.** [Known Issues](#known-issues)  
**10.** [Credits](#credits)

<br />
<div align="center">
  <a href="https://discord.gg/wEyd9w4hG5"><img src="https://img.shields.io/badge/OptiScaler-blue?style=for-the-badge&logo=discord&logoColor=white&logoSize=auto&color=5865F2" alt="Discord invite"></a>
  <a href="https://github.com/optiscaler/OptiScaler/releases/latest"><img src="https://img.shields.io/badge/Download-Stable-green?style=for-the-badge&logo=github&logoSize=auto" alt="Stable release"></a>
  <a href="https://github.com/optiscaler/OptiScaler/releases/tag/nightly"><img src="https://img.shields.io/badge/Download-Nightly-purple?style=for-the-badge&logo=github&logoSize=auto" alt="Nightly release"></a>
  <a href="https://github.com/optiscaler/OptiScaler/wiki"><img src="https://img.shields.io/badge/Documentation-blue?style=for-the-badge&logo=gitbook&logoColor=white&logoSize=auto" alt="Wiki"></a>
</div>

## About

**OptiScaler** is a middleware tool that intercepts supported game upscaler and frame generation calls, then redirects them to the selected backend. It can replace supported DLSS2+ / FSR2+ / XeSS upscaling paths and manage supported frame generation paths in games that already expose the relevant integration points.

> [!CAUTION]
> Do not use this mod with online games or anti-cheat protected modes. It may trigger anti-cheat software and cause bans.

> [!CAUTION]
> Fake websites have presented themselves as the OptiScaler team. The legitimate public places are the GitHub repository, the Discord server, and Nitec's NexusMods page.

Key features inherited from OptiScaler upstream include XeSS, FSR2/FSR3/FSR4-era upscaler paths, DLSS replacement paths, OptiFG, FSR/Xe frame generation routing, Fakenvapi integration, ASI plugin loading, and game-specific quirks.

## Current branch status

This fork branch is `fsr-sdk-22-rr-wukong`. It is an experimental staging branch for:

- FSR SDK 2.2-era integration cleanup.
- FSR Ray Regeneration 1.1 configuration hardening.
- DLSS Ray Reconstruction to FSR Ray Regeneration translation-layer investigation.
- Black Myth: Wukong validation work.
- A proposed FSR-RR Input Doctor diagnostics feature.

Implemented in this branch:

- Hardened FSR-RR denoiser key/index mapping.
- Deterministic FSR-RR state initialization for denoiser context and camera matrices.
- Compile-time guard for FidelityFX denoiser key-count drift.
- Windows build script: `scripts/build-optiscaler.ps1`.
- Batch build wrapper: `build-optiscaler.bat`.
- FSR-RR game report issue template.
- Current branch docs under `docs/`.

Not implemented yet:

- The full upscaler API target bump inside `FSR31Feature_Dx12.cpp`.
- Dedicated Black Myth: Wukong RR quirk wiring in `Quirks.h` / `FSRDFeature_Dx12.cpp`.
- Runtime RR input logging.
- RR Input Doctor overlay/log implementation.
- Verified local MSBuild result from this ChatGPT environment.

See [`docs/documentation-status.md`](docs/documentation-status.md) for the documentation map and current branch truth table.

## How it works

OptiScaler acts as an input/output translation layer:

```text
Game upscaler or frame generation input -> OptiScaler -> selected backend output
```

The in-game setting usually controls the **input** path exposed by the game. The OptiScaler overlay controls the **output** backend selected by the user. Frame generation is similarly split into a source/input path and an output implementation.

Press **Insert** to open the overlay in-game. Press **Page Up** to show the performance stats overlay and **Page Down** to cycle its mode. Keybinds can be customized in the overlay or INI.

![inputs_and_outputs](https://github.com/user-attachments/assets/7ff37fd7-515f-488d-99ff-faa586e206fc)

## Which APIs and Upscalers are Supported?

OptiScaler supports DirectX 11, DirectX 12, and Vulkan, but supported upscalers differ by API and by the game integration.

### DirectX 12

- XeSS
- FSR 2.x / FSR 3.x / FSR 4.x-era paths
- DLSS replacement paths
- FSR Ray Regeneration path under active experimental work in this branch

### DirectX 11

- Native FSR 2.2.1 path
- Native DLSS path
- Native XeSS path where supported
- Some DX12-backed paths through D3D11on12, with compatibility and performance caveats

### Vulkan

- FSR2 / FSR3-era paths
- FSR4-era path via DX12-backed update where supported
- DLSS
- XeSS

### OptiFG + HUDfix

OptiFG is experimental DX12 frame generation support. It can add frame generation to some games without native frame generation, or serve as a fallback when native FG replacement paths do not work correctly. HUD ghosting behavior depends on the game and selected FG path.

## Installation

> [!IMPORTANT]
> For normal user installation steps, use the upstream Wiki and compatibility list first. This branch is experimental and primarily intended for local builds/testing.

For branch-local building, see [`docs/build-optiscaler.md`](docs/build-optiscaler.md).

## Configuration

The legacy configuration reference is [`Config.md`](Config.md). GPU spoofing information is in [`Spoofing.md`](Spoofing.md).

Branch-specific FSR-RR and Black Myth: Wukong caveats are tracked in [`docs/fsr-sdk-22-ray-regeneration-wukong.md`](docs/fsr-sdk-22-ray-regeneration-wukong.md). The proposed diagnostics feature is tracked in [`docs/fsr-rr-input-doctor.md`](docs/fsr-rr-input-doctor.md).

## Build

Requirements:

- Windows 10/11
- Git
- Visual Studio 2022 or Visual Studio Build Tools 2022
- Desktop development with C++ workload
- MSVC v143 toolset
- Windows 10 SDK

Fresh checkout:

```powershell
git clone --recursive -b fsr-sdk-22-rr-wukong https://github.com/phantoslayer21/OptiScaler.git
cd OptiScaler
```

Build Release x64:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-optiscaler.ps1 -Configuration Release -Platform x64 -Clean
```

Or:

```cmd
build-optiscaler.bat -Configuration Release -Platform x64 -Clean
```

Output is copied to:

```text
artifacts\OptiScaler-Release-x64\
```

This branch has not been compiled inside this ChatGPT environment because MSBuild/Visual Studio are unavailable here. Treat a successful local `Release|x64` build as the source of truth.

## Branch documentation

Current branch docs:

- [`docs/documentation-status.md`](docs/documentation-status.md)
- [`docs/build-optiscaler.md`](docs/build-optiscaler.md)
- [`docs/fsr-sdk-22-ray-regeneration-wukong.md`](docs/fsr-sdk-22-ray-regeneration-wukong.md)
- [`docs/fsr-rr-input-doctor.md`](docs/fsr-rr-input-doctor.md)
- [`.github/ISSUE_TEMPLATE/fsr-ray-regeneration-game-report.yml`](.github/ISSUE_TEMPLATE/fsr-ray-regeneration-game-report.yml)

## Known Issues

Check the upstream Wiki compatibility list for known game issues and workarounds. For FSR-RR branch work, use the docs above and file game reports with the FSR-RR issue template.

## Credits

This project is based on [PotatoOfDoom](https://github.com/PotatoOfDoom)'s [CyberFSR2](https://github.com/PotatoOfDoom/CyberFSR2).

Thanks to:

- @PotatoOfDoom for CyberFSR2
- @Artur for DLSS Enabler and NVNGX API work
- @LukeFZ & @Nukem for their mods and shared knowledge
- @FakeMichau for support, testing, and feature work
- @QM for testing efforts and game reach
- @TheRazerMD for testing and support
- @Cryio, @krispy, @krisshietala, @Lordubuntu, @scz, @Veeqo for compatibility matrix work
- The DLSS2FSR community

## License / Third-party credit

This project uses [FreeType](https://gitlab.freedesktop.org/freetype/freetype) licensed under the [FTL](https://gitlab.freedesktop.org/freetype/freetype/-/blob/master/docs/FTL.TXT).

## Sponsors

<table>
 <tbody>
  <tr>
   <td align="center"><img alt="[SignPath]" src="https://avatars.githubusercontent.com/u/34448643" height="30"/></td>
   <td>Free code signing on Windows provided by <a href="https://signpath.io/">SignPath.io</a>, certificate by <a href="https://signpath.org/">SignPath Foundation</a></td>
  </tr>
 </tbody>
</table>
