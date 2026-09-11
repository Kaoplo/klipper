# klipper

klipper is a C++17 desktop screen recorder and replay-buffer application built with Qt 6 Widgets and libobs, the recording backend from OBS Studio. It is an early development project with a working GUI and a separate recording-engine library.

The current implementation targets Linux, with defaults matching Arch Linux/CachyOS. Screen capture prefers PipeWire and has an X11 fallback; audio capture uses OBS's PulseAudio sources. Windows-specific code exists but is incomplete and currently contains compile errors. There is no macOS implementation.

## Current features

- Start and stop file recording from the GUI.
- Start a replay buffer and save recent footage without stopping the buffer.
- Run recording and the replay buffer together, sharing one video/audio encoder pair.
- Capture desktop audio, with optional microphone capture configured in code.
- Generate a timestamped filename on every recording start, adding numeric suffixes when files already exist.
- Change FPS and video bitrate through a basic settings dialog.

There is no live preview, streaming UI, global hotkey support, or persistent settings storage yet.

## Development environment

You need:

| Dependency | Purpose |
| --- | --- |
| GCC or Clang with C++17 support | C++ compiler, including `std::filesystem` |
| CMake 3.19 or newer | Project configuration |
| Ninja or Make | Build execution |
| Qt 6 development files with Widgets | GUI and automatic Qt meta-object generation |
| libobs headers and library | Recording engine |
| OBS plugins, graphics module, data files, and `obs-ffmpeg-mux` | Runtime capture, encoding, and file output |
| pkg-config and GLib development files | Linux GLib event loop used for portal integration |

Qt 6 is required by the current CMake project, even though its package lookup uses `QUIET`. Dependencies are supplied by the system; the repository does not download or vendor them.

### Arch Linux / CachyOS

Install the development dependencies:

```bash
sudo pacman -Syu --needed base-devel git cmake ninja pkgconf qt6-base glib2 obs-studio
```

Arch's native OBS package includes the libobs headers, runtime plugins, and mux helper. See the [package file list](https://archlinux.org/packages/extra/x86_64/obs-studio/files/) for installed paths.

For Wayland capture, use a working PipeWire session and `xdg-desktop-portal` with the backend appropriate to your desktop. OBS lists desktop portal implementations as dependencies for Wayland capture in its [Arch package information](https://archlinux.org/packages/extra/x86_64/obs-studio/). Desktop and microphone audio also need a PulseAudio-compatible service, such as `pipewire-pulse`.

### Other Linux distributions

Install equivalent compiler, CMake, Qt 6 Widgets, GLib, libobs development, and native OBS runtime packages through your distribution. A Flatpak OBS installation alone does not provide the host development environment expected by this build.

Check these defaults in [recording_config.h](src/recording_engine/config/recording_config.h) and change them if your distribution installs OBS elsewhere:

```cpp
std::string plugin_bin_dir = "/usr/lib/obs-plugins";
std::string plugin_data_dir = "/usr/share/obs/obs-plugins";
```

The compiler must be able to resolve `<obs/obs.h>`, and CMake must find the `obs` library. For a custom Qt installation, pass `-DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/compiler` when configuring. The build currently locates libobs with `find_library`, so custom OBS installations may also need explicit compiler include paths and `-DOBS_LIB=/absolute/path/to/libobs.so`.

## Build

Run from the repository root:

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --parallel 2
```

The targets are `recording_engine` (a static library) and `klipper-gui` (the application). Increase the parallel job count to suit your machine. If reusing a build directory configured with another generator, omit `-G Ninja` or choose a fresh build directory.

For an optimized build, use a separate directory:

```bash
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --parallel 2
```

There are no install rules or application packaging targets. Run the executable from its build directory. Extra build directories such as `build-release/` are not currently covered by `.gitignore`; keep their generated files out of commits.

## GitHub Actions builds

The [Build workflow](.github/workflows/build.yml) compiles a Linux x86_64 Release build on pushes to `main` and pull requests targeting `main`. It uses an Arch Linux container to match the current development dependencies and OBS plugin paths. New commits cancel older runs for the same branch or pull request.

Successful runs upload `klipper-linux-x86_64.tar.gz`, retained for 14 days. Download it from the run's **Artifacts** section in GitHub Actions, then extract and run it:

```bash
tar -xzf klipper-linux-x86_64.tar.gz
cd klipper-linux-x86_64/bin
./klipper-gui
```

The archive includes the application executable, the `obs-ffmpeg-mux` helper, artwork, this README, and a `build-packages.txt` inventory of the build environment. The tar archive preserves executable permissions.

This is a dynamically linked development build for an up-to-date Arch Linux/CachyOS system. Install the runtime packages (`qt6-base`, `glib2`, and `obs-studio`) and configure capture services as described above; shared libraries and OBS plugins are not bundled. Other distributions may have incompatible library versions. The container uses rolling Arch packages, with the exact versions recorded in each artifact. CI verifies compilation and packaging; it does not run graphical recording tests or publish GitHub Releases.

## Run

Use a terminal inside your graphical desktop session:

```bash
cd build
./klipper-gui
```

Running from `build/` also makes the current relative icon path, `../assets/512.png`, resolve correctly. For the release build, use `build-release/` instead. Keep the terminal open to see OBS initialization and error logs.

1. Approve the screen-sharing portal dialog and select a screen or window when prompted. The GUI can report “Ready” before permission has been granted.
2. Click **Start Recording** to record to a file, then **Stop Recording** to finish it.
3. Click **Start Replay Buffer**, allow footage to accumulate, then **Save Clip** to write recent footage to disk.
4. Stop the replay buffer when finished. Recording and buffering can run simultaneously.

The keyboard shortcuts work while the main window has focus:

| Shortcut | Action |
| --- | --- |
| `Ctrl+Alt+R` | Toggle recording |
| `Ctrl+Alt+B` | Toggle replay buffer |
| `Ctrl+Alt+S` | Save a replay clip |

They do not work globally while a game or another application has focus.

### Output paths and defaults

Paths are relative to the process's working directory. With the run command above, recordings and replay clips are written under `build/`.

| Setting | Default |
| --- | --- |
| Recording path template | `./Rekording-%CCYY-%MM-%DD %hh-%mm-%ss.mp4` |
| Replay directory | `./klipper_replays` |
| Replay filename template | `Klip-%CCYY-%MM-%DD %hh-%mm-%ss` |
| Replay extension | `mp4` |
| Replay limits | 30 seconds / 500 MB |
| Output video | 1920 × 1080, 60 FPS |
| Video encoder | `obs_x264`, 10,000 kbps |
| Audio encoder | `ffmpeg_aac`, 128 kbps, 48 kHz stereo |
| Desktop audio / microphone | Enabled / disabled |

Recording filenames use OBS date/time placeholders, for example `Rekording-2026-09-08 14-30-00.mp4`. Existing names receive a suffix such as ` (2)` before the extension. The recording template includes its extension; the replay template has a separate extension setting.

The GUI creates the replay directory automatically. If you configure a custom recording directory, create it yourself before recording. Most options currently require editing `RecordingConfig` and rebuilding. The GUI fills the base capture dimensions from Qt's primary screen size.

### Settings limitations

The settings dialog applies FPS and video bitrate by shutting down and reinitializing the engine. Stop both outputs before saving settings; state synchronization during reinitialization is unfinished. Settings last only for the current application session.

The resolution selector is currently a placeholder. Saving settings also enables microphone capture because of temporary testing code in `settings_window.cpp`. The engine currently passes the desktop audio device ID to microphone creation, so selecting a separate microphone device through the config is not wired correctly yet.

## Troubleshooting

| Symptom | What to check |
| --- | --- |
| CMake cannot find Qt or reports a missing `Qt6::Widgets` target | Install Qt 6 development files, or set `CMAKE_PREFIX_PATH` to the Qt installation. |
| `<obs/obs.h>` or the OBS library is missing | Install host libobs development files and check include/library paths. |
| Plugin directory missing, encoder creation fails, or no capture source is available | Check configured plugin paths and the registered source/encoder types printed at startup. Keep libobs and its plugins from a compatible installation. |
| OBS cannot open a display or reset video | Run within a graphical desktop session with working graphics drivers. A headless shell is insufficient for normal operation. |
| Blank capture or no portal prompt | Check PipeWire and your desktop portal backend, then approve screen sharing before starting recording. |
| No desktop audio | Check the default output device and PulseAudio-compatible audio service. Audio-source creation failures are logged but do not necessarily abort initialization. |
| Recording or replay output fails | Check directory permissions, available storage, the `obs-ffmpeg` plugin, and mux-helper availability. |

OBS launches `obs-ffmpeg-mux` as a separate process beside the application executable. CMake now finds the installed helper and copies it beside `klipper-gui` on every build (only when its contents differ). Existing helper symlinks are replaced with a regular file. No manual symlink is needed.

If CMake cannot find the helper, locate the installed binary:

```bash
command -v obs-ffmpeg-mux
```

For a custom OBS installation, pass its path when configuring:

```bash
cmake -S . -B build -DOBS_FFMPEG_MUX_EXECUTABLE=/path/to/obs-ffmpeg-mux
```

Use the helper from the same OBS installation as libobs and its plugins. `cmake --install build --prefix /path/to/install` installs both executables together, and the CI archive includes both. Keep them together when moving the application. This bundles the helper only; compatible system OBS plugins, data, Qt, and OBS/FFmpeg shared libraries are still required. Rebuild after updating system OBS to refresh the bundled helper.

## Source layout

```text
CMakeLists.txt                  Build targets and dependencies
src/frontend/                  Qt application, main window, settings, worker thread
src/recording_engine/
  recording_engine.h/.cpp      Public engine entry point and lifecycle
  config/                      RecordingConfig defaults
  core/                        libobs lifecycle, GLib loop, plugin loading
  capture/                     Screen and audio source creation
  encoding/                    Video/audio encoder factories
  output/                      File recording and replay-buffer outputs
src/main.cpp                   Older engine demo; not a current CMake target
assets/                        Application artwork
.clang-tidy                    Static-analysis configuration
```

`MainWindow` queues engine operations to `EngineWorker` on a dedicated `QThread`. `RecordingEngine` owns the OBS context, capture sources, shared encoders, and both outputs. Changes to capture or encoding belong in the engine; GUI behavior belongs in `src/frontend/`.

When using the engine without the GUI, set nonzero base capture dimensions and create the replay directory before calling `initialize()`. The older `src/main.cpp` demo does not currently supply those dimensions.

## Validation and contributing

### Version and build information

The application starts at version `0.1.0`, declared by `project(... VERSION ...)` in `CMakeLists.txt`. Update that value intentionally for a release and tag the corresponding commit, for example `v0.1.0`.

The window title and startup logs show the version, build number, and abbreviated commit hash. Open **About** for the full commit hash and working-tree state, or use **Copy build information** when reporting an issue.

GitHub Actions supplies a build number in `run.attempt` form, such as `142.1`. Local builds use `local`; a modified working tree adds `-dirty` to the displayed hash. Both tracked changes and untracked files count, while Git-ignored build files do not. Builds without Git metadata show `unknown`. Pull-request builds identify the merge commit actually checked out by CI.

CMake regenerates `generated/build_info.h` inside the build directory on each build, updating the file only when its contents change. This keeps commit information current across incremental builds without unnecessary recompilation. To supply a build number manually, configure with `-DKLIPPER_BUILD_NUMBER=142.1`; use `-DKLIPPER_BUILD_NUMBER=local` to reset an existing build directory to local numbering.

### Checks

There is no automated test suite or CTest setup yet. Build changes locally and use a graphical session for a manual smoke check:

1. Start and stop two recordings; confirm distinct files and playable video/audio.
2. Start the replay buffer, save a clip, and verify the resulting file after writing completes.
3. Record while the replay buffer runs and confirm both outputs work.
4. Stop both outputs, change FPS or bitrate, and verify recording after reinitialization.
5. Close the application and check the console for shutdown errors.

Replay saving is asynchronous in OBS; the UI currently reads the last replay path immediately after requesting a save, so its status text may show an empty or previous path. Check the output directory for completion.

For development, open the root CMake project in your IDE or use `build/compile_commands.json` with clangd. A `.clang-tidy` configuration is provided, but static analysis is not automatically run by CMake. Keep changes focused, describe how they were validated, and include relevant console logs and environment details when reporting capture or encoding failures.

Windows support needs further work before build instructions can be provided: current branches include a misspelled capture variable, a missing semicolon in audio capture, and platform-specific path handling to resolve. Hardware encoder modules may be loaded, but the default configuration and settings UI use x264.

## License

This repository currently has no project license file. Third-party dependencies retain their own licenses; no license for klipper itself is declared here.
