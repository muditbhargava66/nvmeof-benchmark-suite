#include "../../include/optimization_engine/config_knowledge_base.h"
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
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Find the first # for comments and truncate
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }

        // Find the equals sign
        size_t equals_pos = line.find('=');
        if (equals_pos != std::string::npos) {
            std::string key = line.substr(0, equals_pos);
            std::string value = line.substr(equals_pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            // Only add if key is not empty
            if (!key.empty()) {
                config_map_[key] = value;
            }
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