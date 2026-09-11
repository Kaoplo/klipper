//
// Created by kaoplo on 2026. 07. 30..
//

#include "recording_engine/encoding/encoder_factory.h"

#include <iostream>

#include "recording_engine/config/recording_config.h"

namespace klipper {

obs_encoder_t *EncoderFactory::createVideoEncoder(const RecordingConfig &config) {
    enumerate_encoders();
    obs_encoder_t *encoder = obs_video_encoder_create(
        config.video_encoder_id.c_str(), "video_encoder", nullptr, nullptr);
    if (!encoder)
        return nullptr;

    obs_data_t *settings = obs_data_create();
    obs_data_set_int(settings, "bitrate", config.video_bitrate_kbps);
    obs_encoder_update(encoder, settings);
    obs_data_release(settings);

    obs_encoder_set_video(encoder, obs_get_video());
    return encoder;
}

obs_encoder_t *EncoderFactory::createAudioEncoder(const RecordingConfig &config) {
    obs_encoder_t *encoder = obs_audio_encoder_create(
        config.audio_encoder_id.c_str(), "audio_encoder", nullptr, 0, nullptr);
    if (!encoder)
        return nullptr;

    obs_data_t *settings = obs_data_create();
    obs_data_set_int(settings, "bitrate", config.audio_bitrate_kbps);
    obs_encoder_update(encoder, settings);
    obs_data_release(settings);

    obs_encoder_set_audio(encoder, obs_get_audio());
    return encoder;
}

void EncoderFactory::enumerate_encoders() {
    size_t index = 0;
    const char *id = nullptr;

    while (obs_enum_encoder_types(index++, &id)) {
        const char *codec = obs_get_encoder_codec(id);
        obs_encoder_type type = obs_get_encoder_type(id);

        std::cout
            << "ID: " << (id ? id: "")
            << " | codec: " << (codec ? codec : "")
            << " | type: "
            << (type == OBS_ENCODER_VIDEO ? "video" : "audio")
            << std::endl;
    }
}
}
