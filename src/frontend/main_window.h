//
// Created by kaoplo on 8/12/26.
//

#pragma once

#include "frontend/engine_worker.h"

#include <QMainWindow>
#include <QThread>

class QLabel;
class QPushButton;

namespace klipper {

    // Very basic window: three buttons (record, replay buffer, save clip)
    // plus in-app keyboard shortcuts. See engine_worker.h for why button
    // clicks get marshaled onto a background thread rather than calling
    // RecordingEngine directly here.
    class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow() override;

    private slots:
        void onInitialized(bool success);
        void onRecordingStateChanged(bool active);
        void onReplayBufferStateChanged(bool active);
        void onReplaySaved(const QString &path);
        void onErrorOccurred(const QString &message);

    private:
        void setupUi();
        void setupShortcuts();

        QThread worker_thread_;
        EngineWorker *worker_ = nullptr;

        QLabel *status_label_ = nullptr;
        QPushButton *record_button_ = nullptr;
        QPushButton *replay_button_ = nullptr;
        QPushButton *save_clip_button_ = nullptr;
    };

} // namespace klipper