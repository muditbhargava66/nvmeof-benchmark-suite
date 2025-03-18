#include "../../include/bottleneck_analysis/bottleneck_detector.h"
#include "../../include/bottleneck_analysis/resource_monitor.h"
#include <iostream>
#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace nvmeof {
namespace bottleneck_analysis {

BottleneckInfo::BottleneckInfo(
    BottleneckType type,
    const std::string& description,
    double severity,
    const std::string& resource_name,
    double resource_usage,
    const std::string& recommendation
)
    : type(type)
    , description(description)
    , severity(severity)
    , resource_name(resource_name)
    , resource_usage(resource_usage)
    , recommendation(recommendation) {
    
    // Validate severity range
    if (severity < 0.0 || severity > 1.0) {
        throw std::invalid_argument("Bottleneck severity must be between 0.0 and 1.0");
    }
}

BottleneckDetector::BottleneckDetector(
    double cpu_threshold,
    double memory_threshold,
    uint64_t network_threshold,
    uint64_t storage_threshold,
    BottleneckDetectionCallback callback
)
    : cpu_threshold_(cpu_threshold)
    , memory_threshold_(memory_threshold)
    , network_threshold_(network_threshold)
    , storage_threshold_(storage_threshold)
    , callback_(callback) {
    
    // Validate thresholds
    if (cpu_threshold < 0.0 || cpu_threshold > 100.0) {
        throw std::invalid_argument("CPU threshold must be between 0.0 and 100.0");
    }
    
    if (memory_threshold < 0.0 || memory_threshold > 100.0) {
        throw std::invalid_argument("Memory threshold must be between 0.0 and 100.0");
    }
    
    if (network_threshold == 0) {
        throw std::invalid_argument("Network threshold must be greater than 0");
    }
    
    if (storage_threshold == 0) {
        throw std::invalid_argument("Storage threshold must be greater than 0");
    }
}

BottleneckDetector::~BottleneckDetector() {
    // Clean up any resources
}

std::vector<BottleneckInfo> BottleneckDetector::DetectBottlenecks(const ResourceUsage& resource_usage) const {
    // Extract relevant metrics from ResourceUsage
    double cpu_usage = resource_usage.cpu_usage_percent;
    double memory_usage = resource_usage.memory_usage_percent;
    
    // Calculate network usage (sum of all interfaces)
    uint64_t network_rx_total = 0;
    uint64_t network_tx_total = 0;
    for (size_t i = 0; i < resource_usage.interfaces.size(); ++i) {
        network_rx_total += resource_usage.rx_bytes[i];
        network_tx_total += resource_usage.tx_bytes[i];
    }
    
    uint64_t network_usage = network_rx_total + network_tx_total;
    
    // Call the other overload
    return DetectBottlenecks(cpu_usage, memory_usage, network_usage, 0);
}

std::vector<BottleneckInfo> BottleneckDetector::DetectBottlenecks(
    double cpu_usage,
    double memory_usage,
    uint64_t network_usage,
    uint64_t storage_usage
) const {
    std::vector<BottleneckInfo> bottlenecks;
    
    // Check CPU bottleneck
    if (cpu_usage >= cpu_threshold_) {
        double severity = std::min(1.0, (cpu_usage - cpu_threshold_) / (100.0 - cpu_threshold_));
        BottleneckInfo info = CreateBottleneckInfo(
            BottleneckType::CPU,
            "High CPU usage detected",
            severity,
            "CPU",
            cpu_usage,
            "Consider optimizing CPU-intensive operations or upgrading CPU"
        );
        
        bottlenecks.push_back(info);
        
        // Invoke callback if provided
        if (callback_) {
            callback_(info);
        }
        
        std::cout << "Bottleneck detected: High CPU usage (" << cpu_usage << "%)" << std::endl;
    }
    
    // Check memory bottleneck
    if (memory_usage >= memory_threshold_) {
        double severity = std::min(1.0, (memory_usage - memory_threshold_) / (100.0 - memory_threshold_));
        BottleneckInfo info = CreateBottleneckInfo(
            BottleneckType::MEMORY,
            "High memory usage detected",
            severity,
            "Memory",
            memory_usage,
            "Consider optimizing memory usage, enabling huge pages, or adding more memory"
        );
        
        bottlenecks.push_back(info);
        
        // Invoke callback if provided
        if (callback_) {
            callback_(info);
        }
        
        std::cout << "Bottleneck detected: High memory usage (" << memory_usage << "%)" << std::endl;
    }
    
    // Check network bottleneck
    if (network_usage >= network_threshold_) {
        double severity = std::min(1.0, static_cast<double>(network_usage - network_threshold_) / network_threshold_);
        BottleneckInfo info = CreateBottleneckInfo(
            BottleneckType::NETWORK,
            "High network usage detected",
            severity,
            "Network",
            static_cast<double>(network_usage),
            "Consider optimizing network operations, increasing TCP buffer sizes, or upgrading network hardware"
        );
        
        bottlenecks.push_back(info);
        
        // Invoke callback if provided
        if (callback_) {
            callback_(info);
        }
        
        std::cout << "Bottleneck detected: High network usage (" << network_usage << " bytes)" << std::endl;
    }
    
    // Check storage bottleneck
    if (storage_usage >= storage_threshold_ && storage_usage > 0) {
        double severity = std::min(1.0, static_cast<double>(storage_usage - storage_threshold_) / storage_threshold_);
        BottleneckInfo info = CreateBottleneckInfo(
            BottleneckType::STORAGE,
            "High storage I/O usage detected",
            severity,
            "Storage",
            static_cast<double>(storage_usage),
            "Consider optimizing I/O patterns, using multiple queues, or upgrading storage devices"
        );
        
        bottlenecks.push_back(info);
        
        // Invoke callback if provided
        if (callback_) {
            callback_(info);
        }
        
        std::cout << "Bottleneck detected: High storage usage (" << storage_usage << " bytes)" << std::endl;
    }
    
    if (bottlenecks.empty()) {
        std::cout << "No bottlenecks detected." << std::endl;
    }
    
    return bottlenecks;
}

void BottleneckDetector::SetCpuThreshold(double threshold) {
    if (threshold < 0.0 || threshold > 100.0) {
        throw std::invalid_argument("CPU threshold must be between 0.0 and 100.0");
    }
    cpu_threshold_ = threshold;
}

void BottleneckDetector::SetMemoryThreshold(double threshold) {
    if (threshold < 0.0 || threshold > 100.0) {
        throw std::invalid_argument("Memory threshold must be between 0.0 and 100.0");
    }
    memory_threshold_ = threshold;
}

void BottleneckDetector::SetNetworkThreshold(uint64_t threshold) {
    if (threshold == 0) {
        throw std::invalid_argument("Network threshold must be greater than 0");
    }
    network_threshold_ = threshold;
}

void BottleneckDetector::SetStorageThreshold(uint64_t threshold) {
    if (threshold == 0) {
        throw std::invalid_argument("Storage threshold must be greater than 0");
    }
    storage_threshold_ = threshold;
}

void BottleneckDetector::SetCallback(BottleneckDetectionCallback callback) {
    callback_ = callback;
}

BottleneckInfo BottleneckDetector::CreateBottleneckInfo(
    BottleneckType type,
    const std::string& description,
    double severity,
    const std::string& resource_name,
    double resource_usage,
    const std::string& recommendation
) const {
    return BottleneckInfo(
        type,
        description,
        severity,
        resource_name,
        resource_usage,
        recommendation
    );
}

}  // namespace bottleneck_analysis
}  // namespace nvmeof