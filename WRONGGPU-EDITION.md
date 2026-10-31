# OptiScaler, WrongGPU edition

This branch is OptiScaler with two changes for the WrongGPU upscaler. It is built from OptiScaler's
2026-10-30 nightly (commit `e7e3872d`, "10.0.0-dev") and is licensed like OptiScaler itself: GPL-3.0
(see `LICENSE`). All credit for OptiScaler goes to its authors.

## Changes on top of `e7e3872d`

1. **Custom upscalers go by their own names.** A custom upscaler (`OptiScaler\custom\<name>\`) is shown
   with its own name: the version box is labelled with the custom upscaler's name, its versions are listed
   without an "FSR" prefix, the FPS overlay, the status line and the shader times show the custom upscaler's
   name, and the FSR-only panels (FSR 4 presets / models / color space, debug view, FSR 3.1 presets, FSR 3
   manual tuning) are hidden for it.
2. **Plugin interface.** A port of OptiScaler's own `plugin-api` branch (commit `f24ebc05` by FakeMichau)
   onto this nightly: an ASI plugin can add its own tab to the menu. The WrongGPU RR plugin uses it for its
   Ray reconstruction box.

Nothing else is changed. WrongGPU itself is a separate program; it is not part of this repository.
