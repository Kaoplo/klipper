//
// Created by kaoplo on 2026. 07. 30..
//

#include "recording_engine/recording_engine.h"
#include "recording_engine/capture/capture_source.h"
#include "recording_engine/encoding/encoder_factory.h"
#include "capture/audio_capture_source.h"

#include <obs/util/bmem.h>
#include <obs/util/platform.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>


namespace klipper {
namespace {
std::string generateRecordingPath(const std::string &path_template) {
    const std::filesystem::path pattern = std::filesystem::u8path(path_template);
    // Format only the filename so directory names remain unchanged. The
    // template already includes the extension, so OBS should not append one.
    const std::unique_ptr<char, decltype(&bfree)> filename(
        os_generate_formatted_filename(nullptr, true, pattern.filename().u8string().c_str()),
        &bfree);
    if (!filename || !*filename)
        return {};

    const auto path = pattern.parent_path() / std::filesystem::u8path(filename.get());
    auto candidate = path;
    for (unsigned int suffix = 2; std::filesystem::exists(candidate); ++suffix) {
        candidate = path.parent_path() / std::filesystem::u8path(
            path.stem().u8string() + " (" + std::to_string(suffix) + ")" + path.extension().u8string());
    }
    return candidate.u8string();
}
}

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
        if (!file_output_->initialize(video_encoder_, audio_encoder_)) {
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

        obs_set_output_source(0, capture_source_);

        if (config_.capture_desktop_audio) {
            desktop_audio_source_ = AudioCaptureSource::createDesktopAudio(config_.desktop_audio_device_id);
            if (desktop_audio_source_) {
                obs_set_output_source(1, desktop_audio_source_);
            } else {
                std::cerr << "failed to create desktop audio source\n";
            }
        }
        if (config_.capture_microphone) {
            mic_source_ = AudioCaptureSource::createMicrophone(config_.desktop_audio_device_id);
            if (mic_source_) {
                obs_set_output_source(2, mic_source_);
            } else {
                std::cerr << "failed to create mic source\n";
            }
        }

        return true;
    }

void RecordingEngine::shutdown() {
    if (!context_.isInitialized())
        return;

    if (recording_) {
        stopRecording();
    }
    if (replay_active_) {
        stopReplayBuffer();
    }
    // TODO: implement this in a cleaner way
    obs_set_output_source(0, nullptr);
    obs_set_output_source(1, nullptr);
    obs_set_output_source(2, nullptr);

    if (file_output_) {
        file_output_->stop();
    }
    if (replay_output_) {
        replay_output_->stop();
    }

    file_output_.reset();
    replay_output_.reset();

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
    if (mic_source_) {
        obs_source_release(mic_source_);
        mic_source_ = nullptr;
    }
    if (desktop_audio_source_) {
        obs_source_release(desktop_audio_source_);
        desktop_audio_source_ = nullptr;
    }

    context_.shutdown();
}

bool RecordingEngine::startRecording() {
    if (!file_output_ || recording_)
        return false;

    try {
        const auto output_path = generateRecordingPath(config_.output_path);
        if (output_path.empty()) {
            std::cerr << "failed to generate recording path\n";
            return false;
        }
        recording_ = file_output_->start(output_path);
    } catch (const std::filesystem::filesystem_error &error) {
        std::cerr << "failed to generate recording path: " << error.what() << '\n';
        return false;
    }
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
