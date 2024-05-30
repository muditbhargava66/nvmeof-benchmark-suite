#pragma once

#include <string>
#include <unordered_map>

namespace nvmeof {
namespace optimization_engine {

class ConfigKnowledgeBase {
public:
    explicit ConfigKnowledgeBase(const std::string& config_file);

    std::string GetConfigValue(const std::string& key) const;

private:
    void LoadConfigFile(const std::string& config_file);

    std::unordered_map<std::string, std::string> config_map_;
};

}  // namespace optimization_engine
}  // namespace nvmeof