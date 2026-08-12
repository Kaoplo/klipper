//
// Created by kaoplo on 8/12/26.
//

#include "frontend/engine_worker.h"

#include <filesystem>

namespace klipper {

EngineWorker::EngineWorker(QObject *parent) : QObject(parent) {}

EngineWorker::~EngineWorker() {
    shutdownEngine();
}
void EngineWorker::initializeEngine() {
   RecordingConfig config;
    // replay buffer output doesn't create the path itself
    std::filesystem::create_directories(config.replay_buffer_directory);

    bool ok = engine_.initialize(config);
    if (!ok) {
        emit errorOccurred("Failed to initialize recording engine - check console.");
    }
    emit initialized(ok);
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
    engine_.shutdown();
}

} // namespace klipper
