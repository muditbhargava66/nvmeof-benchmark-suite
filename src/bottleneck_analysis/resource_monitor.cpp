#include "bottleneck_analysis/resource_monitor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <sys/sysinfo.h>

namespace nvmeof {
namespace bottleneck_analysis {

ResourceMonitor::ResourceMonitor(const std::chrono::milliseconds& interval)
    : interval_(interval), running_(false) {}

void ResourceMonitor::Start() {
    running_ = true;
    monitor_thread_ = std::thread(&ResourceMonitor::MonitorResources, this);
}

void ResourceMonitor::Stop() {
    running_ = false;
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
}

void ResourceMonitor::MonitorResources() {
    while (running_) {
        std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

        // Monitor CPU usage
        double cpu_usage = GetCPUUsage();
        std::cout << "CPU Usage: " << cpu_usage << "%" << std::endl;

        // Monitor memory usage
        size_t total_memory = GetTotalMemory();
        size_t used_memory = GetUsedMemory();
        double memory_usage = static_cast<double>(used_memory) / total_memory * 100.0;
        std::cout << "Memory Usage: " << memory_usage << "%" << std::endl;

        // Monitor network usage
        std::cout << "Network Usage:" << std::endl;
        for (const auto& interface : GetNetworkInterfaces()) {
            uint64_t rx_bytes = GetNetworkBytesReceived(interface);
            uint64_t tx_bytes = GetNetworkBytesSent(interface);
            std::cout << "  - " << interface << ": RX=" << rx_bytes << " bytes, TX=" << tx_bytes << " bytes" << std::endl;
        }

        std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
        std::chrono::milliseconds elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        if (elapsed_time < interval_) {
            std::this_thread::sleep_for(interval_ - elapsed_time);
        }
    }
}

double ResourceMonitor::GetCPUUsage() {
    static uint64_t prev_idle_time = 0, prev_total_time = 0;

    std::ifstream stat_file("/proc/stat");
    if (!stat_file.is_open()) {
        return 0.0;
    }

    uint64_t idle_time = 0, total_time = 0;
    std::string line;
    std::getline(stat_file, line);
    stat_file.close();

    std::istringstream iss(line);
    std::string cpu_label;
    iss >> cpu_label;
    if (cpu_label != "cpu") {
        return 0.0;
    }

    uint64_t user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

    idle_time = idle + iowait;
    total_time = user + nice + system + idle + iowait + irq + softirq + steal;

    uint64_t idle_time_diff = idle_time - prev_idle_time;
    uint64_t total_time_diff = total_time - prev_total_time;

    prev_idle_time = idle_time;
    prev_total_time = total_time;

    if (total_time_diff == 0) {
        return 0.0;
    }

    double cpu_usage = 100.0 * (1.0 - static_cast<double>(idle_time_diff) / total_time_diff);
    return cpu_usage;
}

size_t ResourceMonitor::GetTotalMemory() {
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) == -1) {
        return 0;
    }
    return sys_info.totalram * sys_info.mem_unit;
}

size_t ResourceMonitor::GetUsedMemory() {
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) == -1) {
        return 0;
    }
    return (sys_info.totalram - sys_info.freeram) * sys_info.mem_unit;
}

uint64_t ResourceMonitor::GetNetworkBytesReceived(const std::string& interface) {
    std::ifstream net_dev("/proc/net/dev");
    if (!net_dev.is_open()) {
        return 0;
    }
    std::string line;
    while (std::getline(net_dev, line)) {
        std::istringstream iss(line);
        std::string iface;
        if (iss >> iface) {
            if (iface.back() == ':') {
                iface.pop_back();
                if (iface == interface) {
                    uint64_t rx_bytes;
                    if (iss >> rx_bytes) {
                        net_dev.close();
                        return rx_bytes;
                    }
                }
            }
        }
    }
    net_dev.close();
    return 0;
}

uint64_t ResourceMonitor::GetNetworkBytesSent(const std::string& interface) {
    std::ifstream net_dev("/proc/net/dev");
    if (!net_dev.is_open()) {
        return 0;
    }
    std::string line;
    while (std::getline(net_dev, line)) {
        std::istringstream iss(line);
        std::string iface;
        if (iss >> iface) {
            if (iface.back() == ':') {
                iface.pop_back();
                if (iface == interface) {
                    uint64_t rx_bytes, tx_bytes;
                    if (iss >> rx_bytes >> tx_bytes) {
                        net_dev.close();
                        return tx_bytes;
                    }
                }
            }
        }
    }
    net_dev.close();
    return 0;
}

}  // namespace bottleneck_analysis
}  // namespace nvmeof