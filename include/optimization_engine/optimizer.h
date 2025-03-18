#pragma once

#include <string>

namespace nvmeof {
namespace optimization_engine {

class ConfigKnowledgeBase;

}  // namespace optimization_engine

namespace bottleneck_analysis {
class BottleneckDetector;
}  // namespace bottleneck_analysis

namespace optimization_engine {

class Optimizer {
public:
    Optimizer(const ConfigKnowledgeBase& config_kb, const bottleneck_analysis::BottleneckDetector& bottleneck_detector);

    void OptimizeConfiguration(double cpu_usage, double memory_usage, uint64_t network_usage);

private:
    std::string GetBottleneckType(double cpu_usage, double memory_usage, uint64_t network_usage) const;

    const ConfigKnowledgeBase& config_kb_;
    const bottleneck_analysis::BottleneckDetector& bottleneck_detector_;

    static constexpr double kCpuUsageThreshold = 80.0;
    static constexpr double kMemoryUsageThreshold = 90.0;
    static constexpr uint64_t kNetworkUsageThreshold = 1000000000;  // 1 GB/s
};

}  // namespace optimization_engine
}  // namespace nvmeof