#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace nvmeof {
namespace bottleneck_analysis {

// Forward declaration
struct ResourceUsage;

/**
 * @brief Enumeration of bottleneck types
 */
enum class BottleneckType {
    NONE,       ///< No bottleneck detected
    CPU,        ///< CPU bottleneck
    MEMORY,     ///< Memory bottleneck
    NETWORK,    ///< Network bottleneck
    STORAGE,    ///< Storage bottleneck
    SYSTEM,     ///< System bottleneck (e.g., kernel parameters)
    UNKNOWN     ///< Unknown bottleneck
};

/**
 * @brief Structure containing details about a detected bottleneck
 */
struct BottleneckInfo {
    BottleneckType type;           ///< Type of bottleneck
    std::string description;       ///< Detailed description of the bottleneck
    double severity;               ///< Severity level (0.0 to 1.0, where 1.0 is most severe)
    std::string resource_name;     ///< Name of the specific resource causing the bottleneck
    double resource_usage;         ///< Usage level of the resource
    std::string recommendation;    ///< Recommended action to address the bottleneck

    /**
     * @brief Creates a new BottleneckInfo instance
     * 
     * @param type Type of bottleneck
     * @param description Detailed description of the bottleneck
     * @param severity Severity level (0.0 to 1.0)
     * @param resource_name Name of the specific resource causing the bottleneck
     * @param resource_usage Usage level of the resource
     * @param recommendation Recommended action to address the bottleneck
     */
    BottleneckInfo(
        BottleneckType type,
        const std::string& description,
        double severity,
        const std::string& resource_name,
        double resource_usage,
        const std::string& recommendation
    );
};

/**
 * @brief Callback type for bottleneck detection
 */
using BottleneckDetectionCallback = std::function<void(const BottleneckInfo&)>;

/**
 * @brief Detects performance bottlenecks based on resource usage
 */
class BottleneckDetector {
public:
    /**
     * @brief Constructs a BottleneckDetector with the specified thresholds
     * 
     * @param cpu_threshold CPU usage threshold as a percentage (0-100)
     * @param memory_threshold Memory usage threshold as a percentage (0-100)
     * @param network_threshold Network usage threshold in bytes per second
     * @param storage_threshold Storage usage threshold in bytes per second
     * @param callback Optional callback to be invoked when a bottleneck is detected
     * 
     * @throws std::invalid_argument If thresholds are invalid
     */
    BottleneckDetector(
        double cpu_threshold = 80.0,
        double memory_threshold = 90.0,
        uint64_t network_threshold = 1000000000,  // 1 GB/s
        uint64_t storage_threshold = 500000000,   // 500 MB/s
        BottleneckDetectionCallback callback = nullptr
    );

    /**
     * @brief Destroys the BottleneckDetector
     */
    virtual ~BottleneckDetector();

    /**
     * @brief Detects bottlenecks based on the specified resource usage
     * 
     * @param resource_usage Resource usage information
     * 
     * @return Vector of detected bottlenecks
     */
    virtual std::vector<BottleneckInfo> DetectBottlenecks(const ResourceUsage& resource_usage) const;

    /**
     * @brief Detects bottlenecks based on the specified resource metrics
     * 
     * @param cpu_usage CPU usage as a percentage (0-100)
     * @param memory_usage Memory usage as a percentage (0-100)
     * @param network_usage Network usage in bytes per second
     * @param storage_usage Storage usage in bytes per second
     * 
     * @return Vector of detected bottlenecks
     */
    virtual std::vector<BottleneckInfo> DetectBottlenecks(
        double cpu_usage,
        double memory_usage,
        uint64_t network_usage,
        uint64_t storage_usage = 0
    ) const;

    /**
     * @brief Sets the CPU usage threshold
     * 
     * @param threshold CPU usage threshold as a percentage (0-100)
     * 
     * @throws std::invalid_argument If the threshold is invalid
     */
    void SetCpuThreshold(double threshold);

    /**
     * @brief Sets the memory usage threshold
     * 
     * @param threshold Memory usage threshold as a percentage (0-100)
     * 
     * @throws std::invalid_argument If the threshold is invalid
     */
    void SetMemoryThreshold(double threshold);

    /**
     * @brief Sets the network usage threshold
     * 
     * @param threshold Network usage threshold in bytes per second
     * 
     * @throws std::invalid_argument If the threshold is invalid
     */
    void SetNetworkThreshold(uint64_t threshold);

    /**
     * @brief Sets the storage usage threshold
     * 
     * @param threshold Storage usage threshold in bytes per second
     * 
     * @throws std::invalid_argument If the threshold is invalid
     */
    void SetStorageThreshold(uint64_t threshold);

    /**
     * @brief Sets the bottleneck detection callback
     * 
     * @param callback Callback to be invoked when a bottleneck is detected
     */
    void SetCallback(BottleneckDetectionCallback callback);

private:
    /**
     * @brief Creates a bottleneck information structure
     * 
     * @param type Type of bottleneck
     * @param description Detailed description of the bottleneck
     * @param severity Severity level (0.0 to 1.0)
     * @param resource_name Name of the specific resource causing the bottleneck
     * @param resource_usage Usage level of the resource
     * @param recommendation Recommended action to address the bottleneck
     * 
     * @return BottleneckInfo instance
     */
    BottleneckInfo CreateBottleneckInfo(
        BottleneckType type,
        const std::string& description,
        double severity,
        const std::string& resource_name,
        double resource_usage,
        const std::string& recommendation
    ) const;

    double cpu_threshold_;               ///< CPU usage threshold as a percentage
    double memory_threshold_;            ///< Memory usage threshold as a percentage
    uint64_t network_threshold_;         ///< Network usage threshold in bytes per second
    uint64_t storage_threshold_;         ///< Storage usage threshold in bytes per second
    BottleneckDetectionCallback callback_; ///< Callback for bottleneck detection
};

}  // namespace bottleneck_analysis
}  // namespace nvmeof