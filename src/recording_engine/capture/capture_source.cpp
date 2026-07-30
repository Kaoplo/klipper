//
// Created by kaoplo on 2026. 07. 30..
//

#include "recording_engine/capture/capture_source.h"

#include <vector>

namespace klipper {

obs_source_t *CaptureSource::createScreenCapture(const std::string &preferred_id) {
#if defined(_WIN32)
    std::vector<std::string> candidates = {
        preffered_id,
        "monitor_capture", // desktop duplication api
    };
#else
    std::vector<std::string> candidates = {
        preferred_id,
        "pipewire-desktop-capture-source",  // pipewire source
        "xshm_input",                       //fallback to x11
    };
#endif
    for (auto &id : candidates) {
        obs_source_t *source = obs_source_create(id.c_str(), "ScreenCapture", nullptr, nullptr);
        if (source)
            return source;
    }
    return nullptr;
}
}