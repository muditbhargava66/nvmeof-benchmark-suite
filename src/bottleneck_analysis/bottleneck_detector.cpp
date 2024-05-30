#include "bottleneck_analysis/bottleneck_detector.h"
#include <iostream>

namespace nvmeof {
namespace bottleneck_analysis {

BottleneckDetector::BottleneckDetector(double cpu_threshold, double memory_threshold, uint64_t network_threshold)
    : cpu_threshold_(cpu_threshold), memory_threshold_(memory_threshold), network_threshold_(network_threshold) {}

void BottleneckDetector::DetectBottlenecks(double cpu_usage, double memory_usage, uint64_t network_usage) {
    bool cpu_bottleneck = cpu_usage >= cpu_threshold_;
    bool memory_bottleneck = memory_usage >= memory_threshold_;
    bool network_bottleneck = network_usage >= network_threshold_;

    if (cpu_bottleneck) {
        std::cout << "Bottleneck detected: High CPU usage (" << cpu_usage << "%)" << std::endl;
    }
    if (memory_bottleneck) {
        std::cout << "Bottleneck detected: High memory usage (" << memory_usage << "%)" << std::endl;
    }
    if (network_bottleneck) {
        std::cout << "Bottleneck detected: High network usage (" << network_usage << " bytes)" << std::endl;
    }
    if (!cpu_bottleneck && !memory_bottleneck && !network_bottleneck) {
        std::cout << "No bottlenecks detected." << std::endl;
    }
}

}  // namespace bottleneck_analysis
}  // namespace nvmeof