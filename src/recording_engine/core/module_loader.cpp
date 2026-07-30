//
// Created by kaoplo on 2026. 07. 29..
//
#include "module_loader.h"

#include <obs/obs.h>

#include <cstdio>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace klipper {

namespace {
#if defined(_WIN32)
    constexpr const char *kPluginExtension = ".dll";
#else
    constexpr const char *kPluginExtension = ".so";
#endif
}

    void ModuleLoader::load(const std::string &bin_dir,
                             const std::string &data_root,
                             const std::vector<std::string> &wanted)
{
    printf("ModuleLoader: scanning bin_dir='%s'\n", bin_dir.c_str());

    if (!fs::exists(bin_dir)) {
        fprintf(stderr, "Plugin bin dir not found: %s\n", bin_dir.c_str());
        return;
    }

    size_t entries_seen = 0;
    for (auto &entry : fs::directory_iterator(bin_dir)) {
        entries_seen++;
        if (entry.path().extension() != kPluginExtension)
            continue;

        std::string stem = entry.path().stem().string();
        bool is_wanted = false;
        for (auto &w : wanted) {
            if (stem == w) {
                is_wanted = true;
                break;
            }
        }
        if (!is_wanted)
            continue;

        std::string data_dir = data_root + "/" + stem;
        obs_module_t *module = nullptr;
        int code = obs_open_module(&module, entry.path().c_str(), data_dir.c_str());
        if (code != MODULE_SUCCESS || !module) {
            fprintf(stderr, "Failed to open module %s (code %d)\n", stem.c_str(), code);
            continue;
        }
        if (!obs_init_module(module)) {
            fprintf(stderr, "Failed to init module %s\n", stem.c_str());
        }
    }

}

void ModuleLoader::printRegisteredTypes() {
   const char *id = nullptr;

    printf(" --- registered input source types --- \n");
    for (size_t i = 0; obs_enum_input_types(i, &id); i++) {
        printf("    %s\n", id);
    }

    printf(" --- registered encoder types ---\n");
    for (size_t i = 0; obs_enum_encoder_types(i, &id); i++) {
        printf("    %s\n", id);
    }
    printf("-------------------\n");
}
}
