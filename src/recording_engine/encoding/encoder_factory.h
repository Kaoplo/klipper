//
// Created by kaoplo on 2026. 07. 30..
//

#pragma once

#include "recording_engine/config/recording_config.h"

#include <obs/obs.h>

#include <vector>

namespace klipper {

class EncoderFactory {
public:
    struct Encoder {
        const char* id;
        const char* codec;
        const obs_encoder_type type;
    };

    static obs_encoder_t *createVideoEncoder(const RecordingConfig &config);
    static obs_encoder_t *createAudioEncoder(const RecordingConfig &config);

    static std::vector<Encoder> enumerate_encoders();
};

}