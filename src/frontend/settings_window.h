//
// Created by kaoplo on 8/12/26.
//

#pragma once

#include <QDialog>

#include "recording_engine/config/recording_config.h"

class QLabel;
class QPushButton;
class QTextEdit;

namespace klipper {
class SettingsPopup : public QDialog {
    Q_OBJECT

public:
    explicit SettingsPopup(const RecordingConfig& config, QWidget *parent = nullptr);
signals:
    void configSaved(const RecordingConfig& newConfig);

private:
    RecordingConfig config_;
    void onSaveClicked();
    QPushButton *close_button_ = nullptr;
    QLabel *settings_label_ = nullptr;
    QLabel *some_setting_ = nullptr;
    };



}