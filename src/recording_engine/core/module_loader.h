//
// Created by kaoplo on 2026. 07. 29..
//

#pragma once

#include <string>
#include <vector>

namespace klipper {
// Loads only whitelisted libobs modules
class ModuleLoader {
public:
    static void load(const std::string &bin_dir,
                    const std::string &data_root,
                    const std::vector<std::string> &wanted);

    static void printRegisteredTypes();
};
}