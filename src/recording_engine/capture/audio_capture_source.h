//
// Created by kaoplo on 8/18/26.
//

#pragma once

#include <obs/obs.h>

#include <string>

namespace klipper {
class AudioCaptureSource {
public:
    static obs_source_t *createDesktopAudio(const std::string &device_id = "default");
    static obs_source_t *createMicrophone(const std::string &device_id = "default");
};

} // klipper

