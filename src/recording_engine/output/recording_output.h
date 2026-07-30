//
// Created by kaoplo on 2026. 07. 30..
//

#pragma once

#include <obs/obs.h>

#include <string>

namespace klipper {

class RecordingOutput {
public:
    ~RecordingOutput();

    bool initialize(obs_encoder_t *video_encoder, obs_encoder_t *audio_encoder,
                    const std::string &output_path);
    bool start();
    void stop();

private:
    obs_output_t *output_ = nullptr;

};
}