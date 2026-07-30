//
// Created by kaoplo on 2026. 07. 29..
//

#pragma once

#include <cstdint>
#include <string>

namespace klipper {
    // everything that we want the frontend to configure in the future
struct RecordingConfig {
    // VIDEO
    uint32_t base_width = 2560;
    uint32_t base_height = 1440;
    uint32_t output_width = 1920;
    uint32_t output_height = 1080;
    uint32_t fps_num = 60;
    uint32_t fps_den = 1;

#if defined(_WIN32)
    std::string graphics_module = "libobs-d3d11";
#else
    std::string graphics_module = "libobs-opengl";
#endif

    // AUDIO
    uint32_t sample_rate = 48000;


    // ENCODING
    // for now we only use "obs_x264"
    std::string video_encoder_id = "obs_x264";
    int video_bitrate_kbps = 10000;
    std::string audio_encoder_id = "ffmpeg_aac";
    int audio_bitrate_kbps = 128;

    // CAPTURE
    // for now only pipewire is supported
#if defined(_WIN32)
    std::string preferred_capture_source_id_ = "monitor_capture";
#else
    std::string preferred_capture_source_id = "pipewire-screen-capture-source";
#endif

    // How long to wait before starting capture
#if defined(_WIN32)
    int capture_permission_wait_sec = 0;
#else
    // TODO: in the future wait for the user to approve, not x amount of seconds
    int capture_permission_wait_sec = 5;
#endif

    // OUTPUT
    std::string output_path = "./klipper_recording.mp4";

#if defined(_WIN32)
    std::string plugin_bin_dir = "obs-plugins/64bit";
    std::string plugin_data_dir = "data/obs-plugins";
#else
    // PLUGIN LOCATIONS
    std::string plugin_bin_dir = "/usr/lib/obs-plugins";
    std::string plugin_data_dir = "/usr/share/obs/obs-plugins";
#endif
};

}