//
// Created by kaoplo on 2026. 07. 30..
//
#pragma once

#include "recording_engine/config/recording_config.h"
#include "recording_engine/core/obs_context.h"
#include "recording_engine/output/recording_output.h"
#include "recording_engine/output/replay_buffer_output.h"

#include <obs/obs.h>

#include <memory>

namespace klipper {

    // The single entry point a frontend (or, for now, main.cpp) should
// need. Everything else under recording_engine/ is an implementation
// detail this class wires together.
//
// Recording and the replay buffer are independent states that can run
// at the same time: initialize() creates ONE video/audio encoder pair
// and attaches it to BOTH a RecordingOutput and a ReplayBufferOutput.
// libobs lets multiple outputs share the same obs_encoder_t - each
// just receives a copy of the same encoded packet stream - so running
// both simultaneously doesn't cost a second encode pass. If you ever
// need the replay buffer at different quality settings than the main
// recording, that assumption breaks and each would need its own
// encoder pair instead.
//
//   RecordingConfig config;
//   config.output_path = "...";       // whatever the UI collected
//   RecordingEngine engine;
//   engine.initialize(config);
//   engine.startRecording();
//   engine.startReplayBuffer();       // both run at once
//   ...
//   engine.saveReplay();              // dump buffer to a file, keeps running
//   ...
//   engine.stopRecording();
//   engine.stopReplayBuffer();
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

    // ---- file recording ----
    bool startRecording();
    void stopRecording();
    [[nodiscard]] bool isRecording() const {return recording_;};

    // ---- replay buffer ----
    // independent of recording
    bool startReplayBuffer();
    void stopReplayBuffer();
    [[nodiscard]] bool isReplayBufferActive() const { return replay_active_; }

    // dumps the current replay buffer contents to a file
    [[nodiscard]] bool saveReplay() const;
    [[nodiscard]] std::string lastReplayPath() const;

private:
    RecordingConfig config_;
    ObsContext context_;

    obs_source_t *capture_source_ = nullptr;
    obs_encoder_t *video_encoder_ = nullptr;
    obs_encoder_t *audio_encoder_ = nullptr;

    std::unique_ptr<RecordingOutput> file_output_;
    std::unique_ptr<ReplayBufferOutput> replay_output_;

    bool recording_ = false;
    bool replay_active_ = false;
};
}
