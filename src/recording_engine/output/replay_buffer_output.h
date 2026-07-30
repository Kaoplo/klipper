//
// Created by kaoplo on 2026. 07. 30..
//

#pragma once

#include <obs/obs.h>

#include <string>

namespace klipper {

class ReplayBufferOutput {
public:
    ~ReplayBufferOutput();

    bool initialize(obs_encoder_t *video_encoder, obs_encoder_t *audio_encoder,
                    const std::string &directory, const std::string &format,
                    const std::string &extension, int max_time_sec, int max_size_mb);

    bool start() const;
    void stop() const;

    // dump the contents of the buffer to a file, safe to call repeatedly
    bool saveReplay();

    [[nodiscard]] std::string lastReplayPath() const;

private:
    obs_output_t *output_ = nullptr;
};
} // klipper

