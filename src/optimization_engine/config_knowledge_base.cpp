#include "optimization_engine/config_knowledge_base.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace nvmeof {
namespace optimization_engine {

ConfigKnowledgeBase::ConfigKnowledgeBase(const std::string& config_file) {
    LoadConfigFile(config_file);
}

void ConfigKnowledgeBase::LoadConfigFile(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << config_file << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            config_map_[key] = value;
        }
    }

    file.close();
}

std::string ConfigKnowledgeBase::GetConfigValue(const std::string& key) const {
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        return it->second;
    }
    return "";
}

}  // namespace optimization_engine
}  // namespace nvmeof