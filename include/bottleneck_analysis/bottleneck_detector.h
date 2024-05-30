#pragma once

#include <cstdint>

namespace nvmeof {
namespace bottleneck_analysis {

class BottleneckDetector {
public:
    BottleneckDetector(double cpu_threshold, double memory_threshold, uint64_t network_threshold);

    void DetectBottlenecks(double cpu_usage, double memory_usage, uint64_t network_usage);

private:
    double cpu_threshold_;
    double memory_threshold_;
    uint64_t network_threshold_;
};

}  // namespace bottleneck_analysis
}  // namespace nvmeof