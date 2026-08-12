//
// Created by kaoplo on 2026. 07. 30..
//

#include "recording_engine/output/recording_output.h"

#include <iostream>

namespace klipper {

RecordingOutput::~RecordingOutput() {
    if (output_) {
        obs_output_release(output_);
    }
}

bool RecordingOutput::initialize(obs_encoder_t *video_encoder, obs_encoder_t *audio_encoder,
                                const std::string &output_path) {
    output_ = obs_output_create("ffmpeg_muxer", "FileOutput", nullptr, nullptr);
    if (!output_)
        return false;

    obs_data_t *settings = obs_data_create();
    obs_data_set_string(settings, "path", output_path.c_str());
    obs_output_update(output_, settings);
    obs_data_release(settings);

    obs_output_set_video_encoder(output_, video_encoder);
    obs_output_set_audio_encoder(output_, audio_encoder, 0);

    return true;
}

bool RecordingOutput::start() {
    if (!output_)
        return false;

    if (!obs_output_start(output_)) {
        std::cerr << "failed to start recording: "
                    << obs_output_get_last_error(output_) << std::endl;
        return false;
    }
    return true;
}

void RecordingOutput::stop() {
    if (output_) {
        obs_output_stop(output_);
    }
}
}
