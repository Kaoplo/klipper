//
// Created by kaoplo on 2026. 07. 30..
//

#include "recording_engine/recording_engine.h"
#include "recording_engine/capture/capture_source.h"
#include "recording_engine/encoding/encoder_factory.h"

#include <chrono>
#include <iostream>
#include <thread>

namespace klipper {
    RecordingEngine::~RecordingEngine() {
        shutdown();
    }

    bool RecordingEngine::initialize(const RecordingConfig &config) {
        config_ = config;

        if (!context_.initialize(config_)) {
            return false;
        }

        capture_source_ = CaptureSource::createScreenCapture(config_.preferred_capture_source_id);
        if (!capture_source_) {
            std::cerr << "failed to create capture source\n";
            return false;
        }
        obs_set_output_source(0, capture_source_);

        std::cout << "waiting for screen-share permission dialog...\n";
        std::this_thread::sleep_for(std::chrono::seconds(config_.capture_permission_wait_sec));

        video_encoder_ = EncoderFactory::createVideoEncoder(config_);
        if (!video_encoder_) {
            std::cerr << "failed to create video encoder ('" << config_.video_encoder_id << "')\n";
        }

        audio_encoder_ = EncoderFactory::createAudioEncoder(config_);
        if (!audio_encoder_) {
            std::cerr << "failed to create audio encoder ('" << config_.audio_encoder_id << "')\n";
        }

        if (!video_encoder_ || !audio_encoder_) {
            return false;
        }

        file_output_ = std::make_unique<RecordingOutput>();
        if (!file_output_->initialize(video_encoder_, audio_encoder_, config_.output_path)) {
            std::cerr << "failed to create output\n";
            return false;
        }

        replay_output_ = std::make_unique<ReplayBufferOutput>();
        if (!replay_output_->initialize(video_encoder_, audio_encoder_, config_.replay_buffer_directory,
                                        config_.replay_buffer_format, config_.replay_buffer_extension,
                                        config_.replay_buffer_max_time_sec,
                                        config_.replay_buffer_max_size_mb)) {
            std::cerr << "failed to create replay buffer output\n";
            return false;
        }

        return true;
    }

void RecordingEngine::shutdown() {
    if (recording_) {
        stopRecording();
    }

    file_output_.reset();

    if (audio_encoder_) {
        obs_encoder_release(audio_encoder_);
        audio_encoder_ = nullptr;
    }
    if (video_encoder_) {
        obs_encoder_release(video_encoder_);
        video_encoder_ = nullptr;
    }
    if (capture_source_) {
        obs_source_release(capture_source_);
        capture_source_ = nullptr;
    }

    context_.shutdown();
}

bool RecordingEngine::startRecording() {
    if (!file_output_ || recording_)
        return false;

    recording_ = file_output_->start();
    return recording_;
}

void RecordingEngine::stopRecording() {
    if (file_output_ && recording_) {
        file_output_->stop();
    }
        recording_ = false;
}

bool RecordingEngine::startReplayBuffer() {
    if (!replay_output_ || replay_active_)
        return false;

    replay_active_ = replay_output_->start();
        return replay_active_;
}

void RecordingEngine::stopReplayBuffer() {
    if (replay_output_ && replay_active_) {
        replay_output_->stop();
    }
    replay_active_ = false;
}

bool RecordingEngine::saveReplay() const {
    if (!replay_output_ || !replay_active_)
        return false;
    return replay_output_->saveReplay();
}

std::string RecordingEngine::lastReplayPath() const {
    if (!replay_output_)
        return {};
    return replay_output_->lastReplayPath();
}
}
