#include "../../include/optimization_engine/optimizer.h"
#include "../../include/optimization_engine/config_knowledge_base.h"
#include "../../include/optimization_engine/config_applicator.h"
#include "../../include/bottleneck_analysis/bottleneck_detector.h"
#include <iostream>

namespace nvmeof {
namespace optimization_engine {

Optimizer::Optimizer(const ConfigKnowledgeBase& config_kb, const bottleneck_analysis::BottleneckDetector& bottleneck_detector)
    : config_kb_(config_kb), bottleneck_detector_(bottleneck_detector) {}

void Optimizer::OptimizeConfiguration(double cpu_usage, double memory_usage, uint64_t network_usage) {
    // Detect bottlenecks and get the list
    auto bottlenecks = bottleneck_detector_.DetectBottlenecks(cpu_usage, memory_usage, network_usage, 0);

    // Process each detected bottleneck
    for (const auto& bottleneck : bottlenecks) {
        std::string bottleneck_type;
        
        // Map bottleneck type to config key
        switch (bottleneck.type) {
            case bottleneck_analysis::BottleneckType::CPU:
                bottleneck_type = "cpu_bottleneck";
                break;
            case bottleneck_analysis::BottleneckType::MEMORY:
                bottleneck_type = "memory_bottleneck";
                break;
            case bottleneck_analysis::BottleneckType::NETWORK:
                bottleneck_type = "network_bottleneck";
                break;
            case bottleneck_analysis::BottleneckType::STORAGE:
                bottleneck_type = "storage_bottleneck";
                break;
            default:
                continue; // Skip unknown bottleneck types
        }
        
        // Get and apply the optimization config
        std::string optimization_config = config_kb_.GetConfigValue(bottleneck_type);
        if (!optimization_config.empty()) {
            std::cout << "Optimization config for " << bottleneck_type << ": " << optimization_config << std::endl;
            // Apply the optimization config using ConfigApplicator
            ConfigApplicator config_applicator;
            config_applicator.ApplyConfiguration(optimization_config);
        }
    }
    
    // If no bottlenecks detected, fall back to the old approach
    if (bottlenecks.empty()) {
        std::string bottleneck_type = GetBottleneckType(cpu_usage, memory_usage, network_usage);
        if (!bottleneck_type.empty()) {
            std::string optimization_config = config_kb_.GetConfigValue(bottleneck_type);
            if (!optimization_config.empty()) {
                std::cout << "Optimization config for " << bottleneck_type << ": " << optimization_config << std::endl;
                // Apply the optimization config using ConfigApplicator
                ConfigApplicator config_applicator;
                config_applicator.ApplyConfiguration(optimization_config);
            }
        }
    }
}

std::string Optimizer::GetBottleneckType(double cpu_usage, double memory_usage, uint64_t network_usage) const {
    if (cpu_usage >= kCpuUsageThreshold) {
        return "cpu_bottleneck";
    } else if (memory_usage >= kMemoryUsageThreshold) {
        return "memory_bottleneck";
    } else if (network_usage >= kNetworkUsageThreshold) {
        return "network_bottleneck";
    }
    return "";
}

}  // namespace optimization_engine
}  // namespace nvmeof