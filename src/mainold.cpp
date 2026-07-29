#include <csignal>
#include <iostream>
#include <atomic>
#include <string>
#include <chrono>
#include <thread>
#include <obs/obs.h>

std::atomic<bool> g_running{true};

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout<< "stopping rec\n";
        g_running = false;
    }
}

void load_essential_modules() {
    // Standard installation paths for OBS plugins on Arch/CachyOS
    const char* base_lib_path = "/usr/lib/obs-plugins/";
    const char* base_data_path = "/usr/share/obs/obs-plugins/";

    const char* essential_modules[] = {
        "obs-ffmpeg",
        "obs-screencast"
    };

    for (const char* mod_name : essential_modules) {
        // Construct the full paths: e.g., /usr/lib/obs-plugins/obs-ffmpeg.so
        std::string lib_path = std::string(base_lib_path) + mod_name + ".so";
        std::string data_path = std::string(base_data_path) + mod_name;

        obs_module_t* module = nullptr;

        // 0 indicates success in libobs
        if (obs_open_module(&module, lib_path.c_str(), data_path.c_str()) == 0) {
            // obs_init_module returns true on success
            if (!obs_init_module(module)) {
                std::cerr << "Failed to initialize module logic for: " << mod_name << "\n";
            } else {
                std::cout << "Module loaded and initialized successfully: " << mod_name << "\n";
            }
        } else {
            std::cerr << "Failed to find or load module: " << mod_name << "\n";
            std::cerr << "  -> Checked path: " << lib_path << "\n";
        }
    }
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::cout << "initializing libobs\n";

    if (!obs_startup("en_US", nullptr, nullptr)) {
        std::cerr<< "failed to initalize libobs core\n";
        return 1;
    }
    std::cout << "libobs initialized successfully\n";
    load_essential_modules();
    std::cout << "loaded obs modules\n";


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
        std::cerr << "failed to reset video\n";
        obs_shutdown();
        return 1;
    }

    obs_source_t* capture_source = obs_source_create("pipewire-screen-capture-source", "ScreenCapture", nullptr, nullptr);
    if (!capture_source) {
        capture_source = obs_source_create("xshm-input", "ScreenCapture", nullptr, nullptr);
    }
    if (!capture_source) {
        std::cerr << "failed to create capture source\n";
        obs_shutdown();
        return 1;
    }

    obs_set_output_source(0, capture_source);

    obs_encoder_t* video_encoder = obs_video_encoder_create("ffmpeg_nvenc", "NVENC_Encoder", nullptr, nullptr);

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

    std::cout<< "starting screen recording to ./klipper_recording.mp4\n";
    std::cout<< "press Ctrl+C to stop\n";

    if (!obs_output_start(file_output)) {
        std::cerr << "failed to start screen recording\n";
    } else {
        while (g_running && obs_output_active(file_output)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    std::cout << "stopping recording and saving\n";
    obs_output_stop(file_output);

    obs_source_release(capture_source);
    obs_encoder_release(video_encoder);
    obs_output_release(file_output);

    obs_shutdown();
    std::cout << "finished recording and saving\n";

    return 0;
}