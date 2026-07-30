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


        output_ = std::make_unique<RecordingOutput>();
        if (!output_->initialize(video_encoder_, audio_encoder_, config_.output_path)) {
            std::cerr << "failed to create output\n";
            return false;
        }

        return true;
    }

void RecordingEngine::shutdown() {
    if (recording_) {
        stopRecording();
    }

    output_.reset();

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
    if (!output_ || recording_)
        return false;

    recording_ = output_->start();
    return recording_;
}

void RecordingEngine::stopRecording() {
    if (output_ && recording_) {
        output_->stop();
    }
        recording_ = false;
}
}
