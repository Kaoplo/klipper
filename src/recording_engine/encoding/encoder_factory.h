//
// Created by kaoplo on 2026. 07. 30..
//

#pragma once

#include "recording_engine/config/recording_config.h"

#include <obs/obs.h>

namespace klipper {

class EncoderFactory {
public:
    static obs_encoder_t *createVideoEncoder(const RecordingConfig &config);
    static obs_encoder_t *createAudioEncoder(const RecordingConfig &config);
};

}