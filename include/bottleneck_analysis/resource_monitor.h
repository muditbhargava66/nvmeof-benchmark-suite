#pragma once

#include <chrono>
#include <thread>
#include <string>
#include <vector>

namespace nvmeof {
namespace bottleneck_analysis {

class ResourceMonitor {
public:
    explicit ResourceMonitor(const std::chrono::milliseconds& interval);
    ~ResourceMonitor();

    void Start();
    void Stop();

private:
    void MonitorResources();
    double GetCPUUsage();
    size_t GetTotalMemory();
    size_t GetUsedMemory();
    uint64_t GetNetworkBytesReceived(const std::string& interface);
    uint64_t GetNetworkBytesSent(const std::string& interface);
    std::vector<std::string> GetNetworkInterfaces();

    std::chrono::milliseconds interval_;
    bool running_;
    std::thread monitor_thread_;
};

}  // namespace bottleneck_analysis
}  // namespace nvmeof