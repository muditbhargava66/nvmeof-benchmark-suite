#pragma once

#include <string>

namespace nvmeof {
namespace optimization_engine {

class ConfigApplicator {
public:
    void ApplyConfiguration(const std::string& config);

private:
    void SetCPUGovernor(const std::string& governor);
    void EnableHugePages(size_t num_pages);
    void SetIRQAffinity(const std::string& irq_affinity);
    void SetTCPRMem(const std::string& rmem_values);
    void SetTCPWMem(const std::string& wmem_values);
};

}  // namespace optimization_engine
}  // namespace nvmeof