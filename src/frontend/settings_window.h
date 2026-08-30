//
// Created by kaoplo on 8/12/26.
//

#pragma once

#include <QDialog>
#include <QFormLayout>

#include "recording_engine/config/recording_config.h"

class QLabel;
class QPushButton;
class QTextEdit;
class QComboBox;
class QSpinBox;

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
    QFormLayout *form_ = nullptr;
    QPushButton *save_button_= nullptr;
    QPushButton *close_button_ = nullptr; // close without saving

    // settings
    QComboBox *res_opt_ = nullptr; // all these have their labels created by the form layout
    QSpinBox *fps_opt_ = nullptr;
    QSpinBox *bitrate_opt_ = nullptr;
    // still add audio settings
    };



}