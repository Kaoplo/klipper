//
// Created by kaoplo on 8/12/26.
//

#include "frontend/settings_window.h"
#include "frontend/main_window.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace klipper {

SettingsPopup::SettingsPopup(const RecordingConfig& config ,QWidget *parent) : QDialog(parent) {
    config_ = config;
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle("Settings");
    resize(300, 150);

    settings_label_ = new QLabel("Recording settings.", this);
    some_setting_ = new QLabel(config_.graphics_module.data(), this);
    close_button_ = new QPushButton("Save and close", this);
    config_.capture_microphone = true;

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(settings_label_);
    layout->addWidget(close_button_);

    connect(close_button_, &QPushButton::clicked, this, [this]() {
        onSaveClicked();
    });
}

void SettingsPopup::onSaveClicked() {
    emit configSaved(config_);
    accept();
}

} //namespace klipper