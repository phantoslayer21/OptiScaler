# OptiScaler, WrongGPU edition

This branch is OptiScaler with a few changes for the WrongGPU upscaler. It is built from OptiScaler's
2026-10-30 nightly (commit `e7e3872d`, "10.0.0-dev") and is licensed like OptiScaler itself: GPL-3.0
(see `LICENSE`). All credit for OptiScaler goes to its authors.

## Changes on top of `e7e3872d`

1. **Custom upscalers go by their own names.** A custom upscaler (`OptiScaler\custom\<name>\`) is shown
   with its own name: the version box is labelled with the custom upscaler's name, its versions are listed
   without an "FSR" prefix, the FPS overlay, the status line and the shader times show the custom upscaler's
   name, and the FSR-only panels (FSR 4 presets / models / color space, debug view, FSR 3.1 presets, FSR 3
   manual tuning) are hidden for it.
2. **Plugin interface.** A port of OptiScaler's own `plugin-api` branch (commit `f24ebc05` by FakeMichau)
   onto this nightly: an ASI plugin can add its own tab to the menu.
3. **Plugin interface, version 2.** Two calls appended to that interface: `RegisterCustomCard` (a card at the
   top of the Custom tab, the tab the menu opens on, shown in every upscaler state) and `ReportUpscaler` (an
   upscaler a plugin runs itself, ray reconstruction for example, reports its name and GPU time; the FPS
   overlay, the footer and the sidebar show it while none of OptiScaler's own upscalers runs). The WrongGPU RR
   plugin uses them for its Ray reconstruction box and its frame time.
4. **Plugin interface, versions 3 to 5.** Version 3: Streamline routes (a plugin handles named functions of
   the game's Streamline interposer, also for games that load it at run time). Version 4: ray reconstruction
   for games that call NGX directly (`RegisterNgxRayReconstruction`). Version 5:
   `BeginExternalCommandListWork` / `EndExternalCommandListWork` bracket work a plugin records into the game's
   command list; at the end OptiScaler puts back the game's root state (the same restore it does after its own
   upscalers: RestoreComputeSignature / RestoreGraphicSignature / ExtendedStateRestore). Games that do not
   rebind their compute root signature after an upscaler call (RE Engine, Resident Evil Requiem) need it. The
   version 4 provider's evaluate is bracketed the same way.
5. **Diagnostics, off by default.** `[Hotfix] DredDiagnostics=true` (D3D12 DRED breadcrumbs and page-fault
   data, dumped once when the device is removed) and `[Hotfix] NullRootViewFix=true` (null root views logged
   and replaced by a zero buffer).

Nothing else is changed. WrongGPU itself is a separate program; it is not part of this repository.
