# klipper recording engine

## Layout

- `main.cpp` — thin entry point; builds a `RecordingConfig` and drives
  a 10-second test recording through `RecordingEngine`.
- `recording_engine/` — the backend. `recording_engine.h` is the only
  file a frontend should need to include; everything else is an
  implementation detail:
    - `config/` — `RecordingConfig`, the struct a future UI will populate.
    - `core/` — libobs process lifecycle (`ObsContext`) and plugin
      loading (`ModuleLoader`).
    - `capture/` — screen-capture source creation.
    - `encoding/` — video/audio encoder creation.
    - `output/` — the file-recording output (`RecordingOutput`). A future
      `output/replay_buffer_output.h/.cpp` would sit alongside this and
      reuse the same capture/encoding code.

## Build

```bash
mkdir build && cd build
cmake ..
make
```

## Known environment caveats (carried over from earlier debugging)

- **`obs-ffmpeg-mux` helper binary**: `ffmpeg_muxer` spawns a helper
  process it expects to find in the *same directory as your built
  executable*, not the plugins folder. Symlink it in (adjust the
  source path to wherever `find /usr -iname 'obs-ffmpeg-mux*'` finds
  it on your system):
  ```bash
  ln -sf /usr/lib/obs-plugins/obs-ffmpeg-mux build/obs-ffmpeg-mux
  ```
  Worth adding as a CMake `POST_BUILD` step once the path is confirmed
  stable on your machine.
- **Plugin directory**: `RecordingConfig::plugin_bin_dir` /
  `plugin_data_dir` default to `/usr/lib/obs-plugins` +
  `/usr/share/obs/obs-plugins` (Arch/CachyOS layout). Adjust for other
  distros.
- **NVENC**: `obs-nvenc` may fail its own hardware self-test depending
  on the machine, in which case no NVENC encoder gets registered and
  `RecordingConfig::video_encoder_id` should stay `"obs_x264"`.
- **PipeWire portal permission dialog**: the first run after startup,
  watch for a screen-share permission popup during the
  `capture_permission_wait_sec` window - it needs to be approved before
  frames actually flow.

## Cross-platform notes

Platform differences are pushed behind `#if defined(_WIN32)` in
exactly three places: `ObsContext` (graphics module, GLib/portal loop,
module whitelist), `CaptureSource` (fallback chain), and
`ModuleLoader` (plugin file extension). `RecordingConfig`'s defaults
already branch per-platform for graphics module, capture source id,
portal wait time, and plugin paths - nothing else in
`recording_engine/` should need its own `#ifdef`.

**Bundling libobs on Windows** is actually simpler than Linux, since
there's no equivalent of "the system already has a competing libobs
install" to work around: grab the `bin/64bit`, `obs-plugins/64bit`,
and `data` folders from an official OBS Studio Windows release/zip and
ship them wholesale alongside your `.exe` (adjust
`plugin_bin_dir`/`plugin_data_dir` to match wherever you lay them out
relative to the executable). Same `obs-ffmpeg-mux.exe`-next-to-your-
binary constraint applies as on Linux.

**Licensing**: libobs, OBS Studio, and libx264 are GPLv2; FFmpeg is
GPL if built with `--enable-gpl` (required for x264 support). Bundling
these into a distributed app generally carries GPL's copyleft
obligations 