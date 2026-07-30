//
// Created by kaoplo on 2026. 07. 30..
//

#include "recording_engine/output/replay_buffer_output.h"

#include <iostream>

#include "recording_engine/capture/capture_source.h"

namespace klipper {

ReplayBufferOutput::~ReplayBufferOutput() {
    if (output_) {
        obs_output_release(output_);
    }
}

bool ReplayBufferOutput::initialize(obs_encoder_t *video_encoder, obs_encoder_t *audio_encoder, const std::string &directory, const std::string &format, const std::string &extension, int max_time_sec, int max_size_mb) {
    output_ = obs_output_create("replay_buffer", "ReplayBuffer", nullptr, nullptr);
    if (!output_)
        return false;

    obs_data_t *settings = obs_data_create();
    obs_data_set_string(settings, "directory", directory.c_str());
    obs_data_set_string(settings, "format", format.c_str());
    obs_data_set_string(settings, "extension", extension.c_str());
    obs_data_set_int(settings, "max_time_sec", max_time_sec);
    obs_data_set_int(settings, "max_size_mb", max_size_mb);
    obs_output_update(output_, settings);
    obs_data_release(settings);

    obs_output_set_video_encoder(output_, video_encoder);
    obs_output_set_audio_encoder(output_, audio_encoder, 0);

    return true;
}

bool ReplayBufferOutput::start() const {
    if (!output_)
        return false;

    if (!obs_output_start(output_)) {
        std::cerr << "failed to start replay buffer: "
                    << obs_output_get_last_error(output_) << std::endl;
        return false;
    }
    return true;
}

void ReplayBufferOutput::stop() const {
    if (output_) {
        obs_output_stop(output_);
    }
}

bool ReplayBufferOutput::saveReplay() {
    if (!output_)
        return false;

    proc_handler_t *ph = obs_output_get_proc_handler(output_);
    calldata_t cd = {};
    bool ok = proc_handler_call(ph, "save", &cd);
    calldata_free(&cd);
    return ok;
}

std::string ReplayBufferOutput::lastReplayPath() const {
    if (!output_)
        return {};
    proc_handler_t *ph = obs_output_get_proc_handler(output_);
    calldata_t cd = {};
    proc_handler_call(ph, "get_last_replay", &cd);
    const char *path = calldata_string(&cd, "path");
    std::string result = path ? path : "";
    calldata_free(&cd);
    return result;
}
} // klipper