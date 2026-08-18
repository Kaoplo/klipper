//
// Created by kaoplo on 8/18/26.
//

#include "recording_engine/capture/audio_capture_source.h"

namespace klipper {

namespace {
obs_source_t *createAudioSource(const char *source_id, const char *name,
                                const std::string &device_id) {
    obs_data_t *settings = obs_data_create();
    obs_data_set_string(settings, "device_id", device_id.c_str());
    obs_source_t *source = obs_source_create(source_id, name, settings, nullptr);
    obs_data_release(settings);
    return source;
}
} // namespace

obs_source_t *AudioCaptureSource::createDesktopAudio(const std::string &device_id) {
#if defined(_WIN32)
    return createAudioSource("wasapi_output_capture", "DesktopAudio", device_id)
#else
    return createAudioSource("pulse_output_capture", "DesktopAudio", device_id);
#endif
}

obs_source_t *AudioCaptureSource::createMicrophone(const std::string &device_id) {
#if defined(_WIN32)
    return createAudioSource("wasapi_input_capture", "Microphone", device_id);
#else
    return createAudioSource("pulse_input_capture", "Microphone", device_id);
#endif
}
} // klipper