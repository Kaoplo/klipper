//
// Created by kaoplo on 8/12/26.
//

#include "frontend/main_window.h"

#include <QGuiApplication>
#include <QScreen>
#include <QLabel>
#include <QMetaObject>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>
#include <QWidget>
#include <QIcon>

#include "settings_window.h"

namespace klipper {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // temporary code
    const QScreen *screen = QGuiApplication::primaryScreen();
    const QSize pixelSize = screen->size();
    current_config_.base_height = pixelSize.height();
    current_config_.base_width = pixelSize.width();


    setupUi();

    worker_ = new EngineWorker();
    worker_->moveToThread(&worker_thread_);
    connect(&worker_thread_, &QThread::finished, worker_, &QObject::deleteLater);

    connect(worker_, &EngineWorker::initialized, this, &MainWindow::onInitialized);
    connect(worker_, &EngineWorker::recordingStateChanged, this, &MainWindow::onRecordingStateChanged);
    connect(worker_, &EngineWorker::replayBufferStateChanged, this,
            &MainWindow::onReplayBufferStateChanged);
    connect(worker_, &EngineWorker::replaySaved, this, &MainWindow::onReplaySaved);
    connect(worker_, &EngineWorker::errorOccurred, this, &MainWindow::onErrorOccurred);

    worker_thread_.start();

    status_label_->setText("Initializing recording engine (watch for the "
                            "screen-share permission dialog)...");
    QMetaObject::invokeMethod(worker_, "initializeEngine", Qt::QueuedConnection, current_config_);

    setupShortcuts();
}

MainWindow::~MainWindow()
{
    // TODO: shutdown runs twice for some reason, fix later
    QMetaObject::invokeMethod(worker_, "shutdownEngine",Qt::BlockingQueuedConnection);
    worker_thread_.quit();
    worker_thread_.wait();
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    status_label_ = new QLabel("Starting up...", central);
    layout->addWidget(status_label_);

    record_button_ = new QPushButton("Start Recording", central);
    record_button_->setEnabled(false);
    connect(record_button_, &QPushButton::clicked, this, [this]() {
        const char *slot = record_button_->text() == "Start Recording" ? "startRecording" : "stopRecording";
        QMetaObject::invokeMethod(worker_, slot, Qt::QueuedConnection);
    });
    layout->addWidget(record_button_);

    replay_button_ = new QPushButton("Start Replay Buffer", central);
    replay_button_->setEnabled(false);
    connect(replay_button_, &QPushButton::clicked, this, [this]() {
        const char *slot =
            replay_button_->text() == "Start Replay Buffer" ? "startReplayBuffer" : "stopReplayBuffer";
        QMetaObject::invokeMethod(worker_, slot, Qt::QueuedConnection);
    });
    layout->addWidget(replay_button_);

    save_clip_button_ = new QPushButton("Save Clip", central);
    save_clip_button_->setEnabled(false);
    connect(save_clip_button_, &QPushButton::clicked, this, [this]() {
        QMetaObject::invokeMethod(worker_, "saveReplay", Qt::QueuedConnection);
    });
    layout->addWidget(save_clip_button_);

    open_settings_ = new QPushButton("Open Settings", central);
    connect(open_settings_, &QPushButton::clicked, this, [this]() {
        auto *settings_popup = new SettingsPopup(current_config_, this);
        connect(settings_popup, &SettingsPopup::configSaved, this, [this](const RecordingConfig& newConfig) {
            current_config_ = newConfig;
            // TODO: shutdown engine first then restart, also stop recording/replay buffer, or don't let the user save.
            QMetaObject::invokeMethod(worker_, "shutdownEngine",Qt::BlockingQueuedConnection);
            QMetaObject::invokeMethod(worker_, "initializeEngine", Qt::QueuedConnection, current_config_);
        });
        settings_popup->exec();
    });
    layout->addWidget(open_settings_);


    QIcon appIcon("../assets/512.png");
    setWindowIcon(appIcon);
    setCentralWidget(central);
    resize(320, 180);
    setWindowTitle("klipper");
}

void MainWindow::setupShortcuts()
{
    // In-app only - fire while THIS window has focus. Not useful for
    // "hotkey while a fullscreen game has focus"; that needs the
    // GlobalShortcuts xdg-desktop-portal instead (see README).
    auto *record_shortcut = new QShortcut(QKeySequence("Ctrl+Alt+R"), this);
    connect(record_shortcut, &QShortcut::activated, record_button_, &QPushButton::click);

    auto *replay_shortcut = new QShortcut(QKeySequence("Ctrl+Alt+B"), this);
    connect(replay_shortcut, &QShortcut::activated, replay_button_, &QPushButton::click);

    auto *clip_shortcut = new QShortcut(QKeySequence("Ctrl+Alt+S"), this);
    connect(clip_shortcut, &QShortcut::activated, save_clip_button_, &QPushButton::click);
}

void MainWindow::onInitialized(bool success)
{
    if (success) {
        status_label_->setText("Ready.");
        record_button_->setEnabled(true);
        replay_button_->setEnabled(true);
    } else {
        status_label_->setText("Failed to initialize - check the console.");
    }
}

void MainWindow::onRecordingStateChanged(bool active)
{
    record_button_->setText(active ? "Stop Recording" : "Start Recording");
    status_label_->setText(active ? "Recording..." : "Ready.");
}

void MainWindow::onReplayBufferStateChanged(bool active)
{
    replay_button_->setText(active ? "Stop Replay Buffer" : "Start Replay Buffer");
    save_clip_button_->setEnabled(active);
}

void MainWindow::onReplaySaved(const QString &path)
{
    status_label_->setText("Saved clip: " + path);
}

void MainWindow::onErrorOccurred(const QString &message)
{
    status_label_->setText(message);
}

} // namespace klipper