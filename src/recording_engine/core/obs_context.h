//
// Created by kaoplo on 2026. 07. 29..
//

#pragma once

#include "recording_engine/config/recording_config.h"

#include <glib.h>

#include <thread>

namespace klipper {

class ObsContext {
public:
    ~ObsContext();

    bool initialize(const RecordingConfig& config);
    void shutdown();

    bool isInitialized() const { return initialized_; }

private:
    void startGlibMainLoop();
    void stopGlibMainLoop();

    bool initialized_ = false;
    GMainLoop *glib_loop_ = nullptr;
    std::thread glib_thread_;
};
}


