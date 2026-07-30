//
// Created by kaoplo on 2026. 07. 26..
//

#include "recording_engine/recording_engine.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>

int main() {
    klipper::RecordingConfig config;
    // defaults in recording_config.h
    // in future override from the frontend

    // replay buffer wont create this itself.
    std::filesystem::create_directories(config.replay_buffer_directory);

    klipper::RecordingEngine engine;
    if (!engine.initialize(config)) {
        std::cerr << "failed to initialize recording engine\n";
        return 1;
    }

    std::cout << "recording for 10 seconds to " << config.output_path << std::endl;
    if (!engine.startRecording()) {
        std::cerr << "failed to start recording\n";
        return 1;
    }

    std::cout << "starting replay buffer alongside it\n";
    if (!engine.startReplayBuffer()) {
        std::cerr << "failed to start replaybuffer\n";
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "saving a clip mid-recording\n";
    if (engine.saveReplay()) {
        std::cout << "replay saved to " << engine.lastReplayPath() << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "stopping recording and replay buffer\n";
    engine.stopRecording();
    engine.stopReplayBuffer();
    engine.shutdown();

    std::cout << "done\n";
    return 0;
}