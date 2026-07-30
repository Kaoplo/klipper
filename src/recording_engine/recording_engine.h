//
// Created by kaoplo on 2026. 07. 30..
//
#pragma once

#include "recording_engine/config/recording_config.h"
#include "recording_engine/core/obs_context.h"
#include "recording_engine/output/recording_output.h"

#include <obs/obs.h>

#include <memory>

namespace klipper {

// The single entry point a frontend (or, for now, main.cpp) should
// need. Everything else under recording_engine/ is an implementation
// detail this class wires together.
//
//   RecordingConfig config;
//   config.output_path = "...";       // whatever the UI collected
//   RecordingEngine engine;
//   engine.initialize(config);
//   engine.startRecording();
//   ...
//   engine.stopRecording();
//   engine.shutdown();
class RecordingEngine {
public:
    ~RecordingEngine();

    // Boot libobs, load plugins, create the capture source and waits for the screenshare permission dialog
    // if any steps fail return false
    bool initialize(const RecordingConfig &config);

    // Releases everything and shuts libobs down.
    // safe to call even if initizalize partially or fully failed.
    void shutdown();

    bool startRecording();
    void stopRecording();

    bool isRecording() const {return recording_;};

private:
    RecordingConfig config_;
    ObsContext context_;

    obs_source_t *capture_source_ = nullptr;
    obs_encoder_t *video_encoder_ = nullptr;
    obs_encoder_t *audio_encoder_ = nullptr;
    std::unique_ptr<RecordingOutput> output_;

    bool recording_ = false;
};
}
