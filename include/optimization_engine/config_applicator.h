#pragma once

#include <string>

namespace nvmeof {
namespace optimization_engine {

class ConfigApplicator {
public:
    virtual ~ConfigApplicator() = default;
    virtual void ApplyConfiguration(const std::string& config);

    // Make these methods public for testing
    virtual void SetCPUGovernor(const std::string& governor);
    virtual void EnableHugePages(size_t num_pages);
    virtual void SetIRQAffinity(const std::string& irq_affinity);
    virtual void SetTCPRMem(const std::string& rmem_values);
    virtual void SetTCPWMem(const std::string& wmem_values);
    virtual void SetSysctlValue(const std::string& key, const std::string& value);
};

}  // namespace optimization_engine
}  // namespace nvmeof