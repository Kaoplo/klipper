//
// Created by kaoplo on 8/12/26.
//

#include "frontend/engine_worker.h"

#include <filesystem>
#include <iostream>

namespace klipper {

EngineWorker::EngineWorker(QObject *parent) : QObject(parent) {}

EngineWorker::~EngineWorker() {
    shutdownEngine();
}
void EngineWorker::initializeEngine(const RecordingConfig& config) {
    std::cout<<"START START"<<std::endl;
    current_config_ = config;
    // replay buffer output doesn't create the path itself
    std::filesystem::create_directories(current_config_.replay_buffer_directory);

    bool ok = engine_.initialize(current_config_);
    if (!ok) {
        emit errorOccurred("Failed to initialize recording engine - check console.");
    }
    emit initialized(ok);
    std::cout<<"STARTED"<<std::endl;
}

void EngineWorker::startRecording() {
    if (!engine_.startRecording()) {
        emit errorOccurred("Failed to start recording - check console.");
        return;
    }
    emit recordingStateChanged(true);
}

void EngineWorker::stopRecording()
{
    engine_.stopRecording();
    emit recordingStateChanged(false);
}

void EngineWorker::startReplayBuffer()
{
    if (!engine_.startReplayBuffer()) {
        emit errorOccurred("Failed to start the replay buffer.");
        return;
    }
    emit replayBufferStateChanged(true);
}

void EngineWorker::stopReplayBuffer()
{
    engine_.stopReplayBuffer();
    emit replayBufferStateChanged(false);
}

void EngineWorker::saveReplay()
{
    if (!engine_.isReplayBufferActive()) {
        emit errorOccurred("Replay buffer isn't running.");
        return;
    }
    if (!engine_.saveReplay()) {
        emit errorOccurred("Failed to save clip.");
        return;
    }
    emit replaySaved(QString::fromStdString(engine_.lastReplayPath()));
}

void EngineWorker::shutdownEngine()
{
    std::cout<<"SHUTDOWN START"<<std::endl;
    engine_.shutdown();
    std::cout<<"SHUTDOWN"<<std::endl;
}

} // namespace klipper
