#pragma once

#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <functional>
#include <memory>

namespace nvmeof {
namespace bottleneck_analysis {

/**
 * @brief Structure to hold system resource usage data
 */
struct ResourceUsage {
    double cpu_usage_percent;                 ///< CPU usage as a percentage
    size_t total_memory_bytes;                ///< Total system memory in bytes
    size_t used_memory_bytes;                 ///< Used system memory in bytes
    double memory_usage_percent;              ///< Memory usage as a percentage
    std::vector<std::string> interfaces;      ///< Network interface names
    std::vector<uint64_t> rx_bytes;           ///< Received bytes per interface
    std::vector<uint64_t> tx_bytes;           ///< Transmitted bytes per interface
    std::vector<uint64_t> rx_packets;         ///< Received packets per interface
    std::vector<uint64_t> tx_packets;         ///< Transmitted packets per interface
    std::chrono::system_clock::time_point timestamp; ///< Timestamp of the measurement

    /**
     * @brief Creates a new ResourceUsage instance with default values
     */
    ResourceUsage();

    /**
     * @brief Get memory usage as a percentage
     * @return Memory usage as a percentage (0-100)
     */
    double GetMemoryUsagePercent() const;
};

/**
 * @brief Callback type for resource usage monitoring
 */
using ResourceMonitorCallback = std::function<void(const ResourceUsage&)>;

/**
 * @brief Monitors system resources (CPU, memory, network) at specified intervals
 */
class ResourceMonitor {
public:
    /**
     * @brief Constructs a ResourceMonitor with the specified monitoring interval
     * 
     * @param interval Interval between monitoring samples
     * @param callback Optional callback to be invoked with each resource usage sample
     * 
     * @throws std::invalid_argument If the interval is zero
     */
    explicit ResourceMonitor(
        const std::chrono::milliseconds& interval,
        ResourceMonitorCallback callback = nullptr
    );

    /**
     * @brief Destroys the ResourceMonitor, stopping monitoring if it's running
     */
    ~ResourceMonitor();

    /**
     * @brief Starts resource monitoring
     * 
     * @return true if monitoring was started successfully, false otherwise
     * 
     * @throws std::runtime_error If monitoring is already running or cannot be started
     */
    bool Start();

    /**
     * @brief Stops resource monitoring
     * 
     * @return true if monitoring was stopped successfully, false otherwise
     */
    bool Stop();

    /**
     * @brief Checks if resource monitoring is currently running
     * 
     * @return true if monitoring is running, false otherwise
     */
    bool IsRunning() const;

    /**
     * @brief Gets the most recent resource usage measurement
     * 
     * @return The most recent ResourceUsage instance
     */
    ResourceUsage GetLatestUsage() const;

    /**
     * @brief Sets the monitoring interval
     * 
     * @param interval New interval between monitoring samples
     * 
     * @throws std::invalid_argument If the interval is zero
     */
    void SetInterval(const std::chrono::milliseconds& interval);

    /**
     * @brief Gets the current monitoring interval
     * 
     * @return The current interval between monitoring samples
     */
    std::chrono::milliseconds GetInterval() const;

    /**
     * @brief Sets the resource monitor callback
     * 
     * @param callback Callback to be invoked with each resource usage sample
     */
    void SetCallback(ResourceMonitorCallback callback);

private:
    /**
     * @brief Main monitoring function that runs in a separate thread
     */
    void MonitorResources();

    /**
     * @brief Gets the current CPU usage as a percentage
     * 
     * @return CPU usage as a percentage (0-100)
     */
    double GetCPUUsage();

    /**
     * @brief Gets the total system memory in bytes
     * 
     * @return Total system memory in bytes
     */
    size_t GetTotalMemory();

    /**
     * @brief Gets the used system memory in bytes
     * 
     * @return Used system memory in bytes
     */
    size_t GetUsedMemory();

    /**
     * @brief Gets the received bytes for a network interface
     * 
     * @param interface Network interface name
     * 
     * @return Received bytes for the interface
     */
    uint64_t GetNetworkBytesReceived(const std::string& interface);

    /**
     * @brief Gets the transmitted bytes for a network interface
     * 
     * @param interface Network interface name
     * 
     * @return Transmitted bytes for the interface
     */
    uint64_t GetNetworkBytesSent(const std::string& interface);

    /**
     * @brief Gets the received packets for a network interface
     * 
     * @param interface Network interface name
     * 
     * @return Received packets for the interface
     */
    uint64_t GetNetworkPacketsReceived(const std::string& interface);

    /**
     * @brief Gets the transmitted packets for a network interface
     * 
     * @param interface Network interface name
     * 
     * @return Transmitted packets for the interface
     */
    uint64_t GetNetworkPacketsSent(const std::string& interface);

    /**
     * @brief Gets a list of available network interfaces
     * 
     * @return List of network interface names
     */
    std::vector<std::string> GetNetworkInterfaces();

    std::chrono::milliseconds interval_;      ///< Interval between monitoring samples
    std::atomic<bool> running_;               ///< Flag indicating if monitoring is running
    std::thread monitor_thread_;              ///< Thread for resource monitoring
    ResourceMonitorCallback callback_;        ///< Callback for resource usage samples
    mutable std::mutex mutex_;                ///< Mutex for thread safety
    ResourceUsage latest_usage_;              ///< Most recent resource usage measurement

    // For CPU usage calculation
    uint64_t prev_idle_time_;                 ///< Previous idle time for CPU usage calculation
    uint64_t prev_total_time_;                ///< Previous total time for CPU usage calculation
};

}  // namespace bottleneck_analysis
}  // namespace nvmeof