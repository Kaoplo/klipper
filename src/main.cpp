//
// Created by kaoplo on 2026. 07. 26..
//
#include <iostream>
#include <obs/obs.h>
#include <filesystem>
#include <thread>
#include <vector>
#include <glib.h>

namespace fs = std::filesystem;

// linux-pipewire talks to xdg-desktop-portal over D-Bus using GLib's
// GDBus, which is async and only progresses if something iterates a
// GMainContext. Real OBS is a Qt app, and Qt on Linux typically has
// GLib integration built into its event loop, so this "just works"
// there. A bare console app has no event loop at all, so without this,
// the portal session request gets queued but never actually sent/
// processed - source creates fine, but no dialog ever appears and no
// real frames ever arrive.
static GMainLoop *g_glib_loop = nullptr;
static std::thread g_glib_thread;

static void start_glib_main_loop()
{
    GMainContext *ctx = g_main_context_default();
    g_glib_loop = g_main_loop_new(ctx, FALSE);
    g_glib_thread = std::thread([]() {
        g_main_loop_run(g_glib_loop);
    });
}

static void stop_glib_main_loop()
{
    if (g_glib_loop) {
        g_main_loop_quit(g_glib_loop);
        if (g_glib_thread.joinable())
            g_glib_thread.join();
        g_main_loop_unref(g_glib_loop);
        g_glib_loop = nullptr;
    }
}

static const std::vector<std::string> kWantedModules = {
    "linux-capture",
    "obs-x264",
    "obs-ffmpeg",
    "linux-pipewire",
    "obs-nvenc"
};

static void load_essential_modules(const std::string &bin_dir, const std::string &data_root)
{
    if (!fs::exists(bin_dir)) {
        fprintf(stderr, "Plugin bin dir not found: %s\n", bin_dir.c_str());
        return;
    }
    for (auto &entry : fs::directory_iterator(bin_dir)) {
        if (entry.path().extension() != ".so")
            continue;

        std::string stem = entry.path().stem().string();
        bool wanted = false;
        for (auto &w : kWantedModules) {
            if (stem == w) {
                wanted = true;
                break;
            }
        }
        if (!wanted) {
            printf("Skipping module (not in kWantedModules): %s\n", stem.c_str());
            continue;
        }

        std::string data_dir = data_root + "/" + stem;
        obs_module_t *module = nullptr;
        int code = obs_open_module(&module, entry.path().c_str(), data_dir.c_str());
        if (code != MODULE_SUCCESS || !module) {
            fprintf(stderr, "Failed to open module %s (code %d)\n", stem.c_str(), code);
            continue;
        }
        if (!obs_init_module(module)) {
            fprintf(stderr, "Failed to init module %s\n", stem.c_str());
        } else {
            printf("Loaded module: %s\n", stem.c_str());
        }
    }
}

int main() {
    std::cout<<"start";

    if (!obs_startup("en_US", nullptr, nullptr)) {
        std::cerr << "failed to initalize obs core\n";
        return 1;
    }

    std::cout << "obs initialized successfully\n";
    std::cout<<"using obs version... " << obs_get_version_string() << std::endl;
    std::cout<< "loading modules\n";

    start_glib_main_loop();

    // IMPORTANT: obs_reset_video() must happen BEFORE modules are loaded.
    // Real OBS Studio's startup order is: obs_startup -> obs_reset_video /
    // obs_reset_audio -> load modules -> obs_post_load_modules. Some
    // modules (linux-pipewire in particular) touch the graphics context
    // (EGL/OpenGL, DMA-BUF capability checks) as part of their own
    // obs_module_load(), so if no video context exists yet they
    // dereference something null and crash. That's the SIGSEGV you were
    // hitting during "Loading module: linux-pipewire.so".
    obs_video_info ovi = {};
    ovi.graphics_module = "libobs-opengl";
    ovi.fps_num = 60;
    ovi.fps_den = 1;
    ovi.base_width = 2560;
    ovi.base_height = 1440;
    ovi.output_width = 1920;
    ovi.output_height = 1080;
    ovi.output_format = VIDEO_FORMAT_NV12;
    ovi.adapter = 0;
    ovi.gpu_conversion = true;

    int video_result = obs_reset_video(&ovi);
    if (video_result != OBS_VIDEO_SUCCESS) {
        std::cerr << "failed to reset video: " << video_result << "\n";
        obs_shutdown();
        return 1;
    }

    // Required for obs_get_audio() to return anything non-null - without
    // this, no audio_t exists at all, which is why both pointers printed
    // as (nil) below.
    obs_audio_info oai = {};
    oai.samples_per_sec = 48000;
    oai.speakers = SPEAKERS_STEREO;
    if (!obs_reset_audio(&oai)) {
        std::cerr << "failed to reset audio\n";
        obs_shutdown();
        return 1;
    }


    load_essential_modules("/usr/lib/obs-plugins", "/usr/share/obs/obs-plugins");
    obs_post_load_modules();
    {
        const char *id = nullptr;
        printf("---- registered input source types ----\n");
        for (size_t i = 0; obs_enum_input_types(i, &id); i++) {
            printf("  %s\n", id);
        }
        printf("---- registered encoder types ----\n");
        for (size_t i = 0; obs_enum_encoder_types(i, &id); i++) {
            printf("  %s\n", id);
        }
        printf("-----------------------------------------\n");
    }

    std::cout<<"loaded modules successfully\n";

    obs_source_t* capture_source = obs_source_create("pipewire-screen-capture-source", "ScreenCapture", nullptr, nullptr);
    if (!capture_source) {
        capture_source = obs_source_create("pipewire-desktop-capture-source", "ScreenCapture", nullptr, nullptr);
    }
    if (!capture_source) {
        capture_source = obs_source_create("xshm_input", "ScreenCapture", nullptr, nullptr);
    }
    if (!capture_source) {
        std::cerr << "failed to create capture source\n";
        obs_shutdown();
        return 1;
    }

    obs_set_output_source(0, capture_source);

    // Give the portal dialog time to appear and actually be approved
    // before we start the 10-second recording clock - otherwise the
    // approval window and the recording window compete for the same
    // 10 seconds.
    std::cout << "waiting for screen-share permission dialog "
                 "(check for a popup now)...\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // obs-nvenc.so failed its own capability self-test on this machine
    // ("Failed to launch the NVENC test process" -> "NVENC not
    // supported"), so no NVENC encoder ever got registered - see the
    // "registered encoder types" diagnostic output. obs_x264 (software
    // x264) is what's actually available, so use that for now.
    obs_encoder_t* video_encoder = obs_video_encoder_create("obs_x264", "X264_Encoder", nullptr, nullptr);

    obs_data_t* encoder_settings = obs_data_create();
    obs_data_set_int(encoder_settings, "bitrate", 10000);
    obs_encoder_update(video_encoder, encoder_settings);
    obs_data_release(encoder_settings);

    obs_encoder_set_video(video_encoder, obs_get_video());

    obs_output_t* file_output = obs_output_create("ffmpeg_muxer", "FileOutput", nullptr, nullptr);

    obs_data_t* output_settings = obs_data_create();
    obs_data_set_string(output_settings, "path", "./klipper_recording.mp4");
    obs_output_update(file_output, output_settings);
    obs_data_release(output_settings);

    obs_output_set_video_encoder(file_output, video_encoder);

    // Required: ffmpeg_muxer won't start without an audio encoder too.
    obs_encoder_t* audio_encoder = obs_audio_encoder_create("ffmpeg_aac", "AAC_Encoder", nullptr, 0, nullptr);
    obs_output_set_audio_encoder(file_output, audio_encoder, 0);

    // Diagnostic: if obs_output_audio() below prints null, it confirms
    // this output has no audio context wired up by default in this
    // libobs version (32.x's canvas-based media system), which would
    // explain "has no media set" even after obs_encoder_set_audio().
    printf("obs_get_audio(): %p, obs_output_audio(file_output): %p\n",
           (void *)obs_get_audio(), (void *)obs_output_audio(file_output));

    // Call this AFTER attaching to the output, so if
    // obs_output_set_audio_encoder() above touched/cleared the
    // encoder's media pointer, this explicit assignment wins instead.
    obs_encoder_set_audio(audio_encoder, obs_get_audio());

    std::cout<< "starting screen recording to ./klipper_recording.mp4\n";
    std::cout<< "press Ctrl+C to stop\n";

    if (!obs_output_start(file_output)) {
        std::cerr << "failed to start screen recording: "
                   << obs_output_get_last_error(file_output) << "\n";
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::cout << "stopping recording and saving\n";
    obs_output_stop(file_output);

    obs_source_release(capture_source);
    obs_encoder_release(video_encoder);
    obs_encoder_release(audio_encoder);
    obs_output_release(file_output);

    stop_glib_main_loop();
    obs_shutdown();
    std::cout << "finished recording and saving\n";


    return 0;
}