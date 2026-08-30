//
// Created by kaoplo on 2026. 07. 29..
//
#include "recording_engine/core/obs_context.h"
#include "recording_engine/core/module_loader.h"
#include "recording_engine/config/recording_config.h"

#include <obs/obs.h>

#include <cstdio>
#include <iostream>
#include <vector>


namespace klipper {
namespace {
#if defined(_WIN32)
    const std::vector<std::string> kWantedModules = {
        "win-capture",
        "win-wasapi",
        "obs-x264",
        "obs-ffmpeg",
        "obs-nvenc",
        "obs-qsv11"
    };
#else
    const std::vector<std::string> kWantedModules = {
        "linux-capture",
        "linux-pipewire",
        "linux-pulseaudio",
        "obs-x264",
        "obs-ffmpeg",
        "obs-nvenc",
    };
#endif
}

ObsContext::~ObsContext() {
    if (initialized_) {
        shutdown();
    }
}
#if !defined(_WIN32)
void ObsContext::startGlibMainLoop()
{
    GMainContext *ctx = g_main_context_default();
    glib_loop_ = g_main_loop_new(ctx, FALSE);
    glib_thread_ = std::thread([this]() {
        g_main_loop_run(glib_loop_);
    });
}

void ObsContext::stopGlibMainLoop() {
    if (glib_loop_) {
        g_main_loop_quit(glib_loop_);
        if (glib_thread_.joinable())
            glib_thread_.join();
        g_main_loop_unref(glib_loop_);
        glib_loop_ = nullptr;
    }
}
#else
void ObsContext::startGlibMainLoop() {}
void ObsContext::stopGlibMainLoop() {}
#endif

bool ObsContext::initialize(const RecordingConfig &config) {
    if (!obs_startup("en_US", nullptr, nullptr)) {
        std::cerr << "failed to initialize obs core\n";
        return false;
    }

    startGlibMainLoop();

    // obs_reset_video/audio MUST happen before modules load
    // some modules need the graphics context to already exist
    obs_video_info ovi = {};
    ovi.graphics_module = config.graphics_module.c_str();
    ovi.fps_num = config.fps_num;
    ovi.fps_den = config.fps_den;
    ovi.base_width = config.base_width;
    ovi.base_height = config.base_height;
    ovi.output_width = config.output_width;
    ovi.output_height = config.output_height;
    ovi.output_format = VIDEO_FORMAT_NV12;
    ovi.adapter = 0; // for now we use the first adapter for transcoding. could implement changing this in the future
    ovi.gpu_conversion = true;

    if (obs_reset_video(&ovi) != OBS_VIDEO_SUCCESS) {
        std::cerr << "failed to reset video\n";
        return false;
    }

    obs_audio_info oai = {};
    oai.samples_per_sec = config.sample_rate;
    oai.speakers = SPEAKERS_STEREO;

    if (!obs_reset_audio(&oai)) {
        std::cerr << "failed to reset audio\n";
        return false;
    }

    ModuleLoader::load(config.plugin_bin_dir, config.plugin_data_dir, kWantedModules);
    obs_post_load_modules();
    ModuleLoader::printRegisteredTypes();

    initialized_ = true;
    return true;
}

void ObsContext::shutdown() {
    if (!initialized_)
        return;
    stopGlibMainLoop();
    obs_shutdown();
    initialized_ = false;
}
}

