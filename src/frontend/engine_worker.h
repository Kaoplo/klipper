//
// Created by kaoplo on 8/12/26.
//

#pragma once

#include "recording_engine/recording_engine.h"

#include <QObject>
#include <QString>

namespace klipper {
class EngineWorker : public QObject {
    Q_OBJECT

public:
    explicit EngineWorker(QObject *parent = nullptr);
    ~EngineWorker() override;

public slots:
    void initializeEngine(const RecordingConfig&);
    void startRecording();
    void stopRecording();
    void startReplayBuffer();
    void stopReplayBuffer();
    void saveReplay();
    void shutdownEngine();

signals:
    void initialized(bool success);
    void recordingStateChanged(bool active);
    void replayBufferStateChanged(bool active);
    void replaySaved(const QString& path);
    void errorOccurred(const QString& message);

private:
    RecordingEngine engine_;
    RecordingConfig current_config_;
};
} // namespace klipper