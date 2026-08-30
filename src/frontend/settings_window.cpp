//
// Created by kaoplo on 8/12/26.
//

#include "frontend/settings_window.h"
#include "frontend/main_window.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>

namespace klipper {

SettingsPopup::SettingsPopup(const RecordingConfig& config ,QWidget *parent) : QDialog(parent) {
    config_ = config;
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle("Settings");
    resize(300, 150);
    form_ = new QFormLayout(this);

    // this does not change anything rn, just a placeholder
    // TODO: have a struct in the future to select resolutions and values
    res_opt_ = new QComboBox(this);
    res_opt_->setPlaceholderText("2560x1440");
    res_opt_->addItem("1920x1080");
    res_opt_->addItem("1280x720");

    fps_opt_ = new QSpinBox(this);
    fps_opt_->setRange(5,240);
    fps_opt_->setSingleStep(30);
    fps_opt_->setValue(config_.fps_num);
    fps_opt_->setSuffix(" fps");

    bitrate_opt_ = new QSpinBox(this);
    bitrate_opt_->setRange(100,100000);
    bitrate_opt_->setSingleStep(100);
    bitrate_opt_->setValue(config_.video_bitrate_kbps);
    bitrate_opt_->setSuffix(" kbps");

    close_button_ = new QPushButton("Close without saving", this);
    save_button_ = new QPushButton("Save and close", this);

    form_->addRow("Resolution: ", res_opt_);
    form_->addRow("FPS: ", fps_opt_);
    form_->addRow("Bitrate: ", bitrate_opt_);
    form_->addRow(close_button_, save_button_);

    // just for testing
    config_.capture_microphone = true;

    connect(save_button_, &QPushButton::clicked, this, [this]() {
        onSaveClicked();
    });
    connect(close_button_, &QPushButton::clicked, this, [this]() {
        accept();
    });
}

void SettingsPopup::onSaveClicked() {
    config_.fps_num = fps_opt_->value();
    config_.video_bitrate_kbps = bitrate_opt_->value();
    emit configSaved(config_);
    accept();
}

} //namespace klipper