//
// Created by kaoplo on 2026. 07. 30..
//

#pragma once

#include <obs/obs.h>

#include <string>

namespace klipper {

class CaptureSource {
public:
    static obs_source_t *createScreenCapture(const std::string &preferred_id);
};
}
