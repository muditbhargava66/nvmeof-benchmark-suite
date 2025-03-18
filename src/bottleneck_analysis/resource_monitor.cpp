#include "../../include/bottleneck_analysis/resource_monitor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>

// Platform-specific includes
#ifdef __APPLE__
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#else
#include <sys/sysinfo.h>
#endif

namespace nvmeof {
namespace bottleneck_analysis {

ResourceUsage::ResourceUsage() 
    : cpu_usage_percent(0.0),
      total_memory_bytes(0),
      used_memory_bytes(0),
      memory_usage_percent(0.0),
      timestamp(std::chrono::system_clock::now()) {}

double ResourceUsage::GetMemoryUsagePercent() const {
    if (total_memory_bytes == 0) {
        return 0.0;
    }
    
    double percentage = static_cast<double>(used_memory_bytes) / total_memory_bytes * 100.0;
    return std::min(percentage, 100.0);  // Cap at 100%
}

ResourceMonitor::ResourceMonitor(
    const std::chrono::milliseconds& interval,
    ResourceMonitorCallback callback)
    : interval_(interval),
      running_(false),
      callback_(callback),
      prev_idle_time_(0),
      prev_total_time_(0) {
    
    if (interval_.count() == 0) {
        throw std::invalid_argument("Monitoring interval cannot be zero");
    }
}

ResourceMonitor::~ResourceMonitor() {
    // Ensure we stop monitoring before destroying the object
    Stop();
}

bool ResourceMonitor::Start() {
    if (running_) {
        throw std::runtime_error("Resource monitor is already running");
    }
    
    running_ = true;
    monitor_thread_ = std::thread(&ResourceMonitor::MonitorResources, this);
    
    return true;
}

bool ResourceMonitor::Stop() {
    if (!running_) {
        return false;
    }
    
    running_ = false;
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
    
    return true;
}

void ResourceMonitor::MonitorResources() {
    while (running_) {
        std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

        // Monitor CPU usage
        double cpu_usage = GetCPUUsage();
        
        // Monitor memory usage
        size_t total_memory = GetTotalMemory();
        size_t used_memory = GetUsedMemory();
        double memory_usage = static_cast<double>(used_memory) / total_memory * 100.0;
        
        // Update latest usage
        {
            std::lock_guard<std::mutex> lock(mutex_);
            latest_usage_.cpu_usage_percent = cpu_usage;
            latest_usage_.total_memory_bytes = total_memory;
            latest_usage_.used_memory_bytes = used_memory;
            latest_usage_.memory_usage_percent = memory_usage;
            latest_usage_.timestamp = std::chrono::system_clock::now();
            
            // Monitor network usage
            latest_usage_.interfaces = GetNetworkInterfaces();
            latest_usage_.rx_bytes.clear();
            latest_usage_.tx_bytes.clear();
            latest_usage_.rx_packets.clear();
            latest_usage_.tx_packets.clear();
            
            for (const auto& interface : latest_usage_.interfaces) {
                latest_usage_.rx_bytes.push_back(GetNetworkBytesReceived(interface));
                latest_usage_.tx_bytes.push_back(GetNetworkBytesSent(interface));
                latest_usage_.rx_packets.push_back(GetNetworkPacketsReceived(interface));
                latest_usage_.tx_packets.push_back(GetNetworkPacketsSent(interface));
            }
        }
        
        // Call the callback if provided
        if (callback_) {
            callback_(latest_usage_);
        }
        
        // Print monitoring info if in verbose mode
        if (false) {  // Set to true for debugging
            std::cout << "CPU Usage: " << cpu_usage << "%" << std::endl;
            std::cout << "Memory Usage: " << memory_usage << "%" << std::endl;
            std::cout << "Network Usage:" << std::endl;
            for (size_t i = 0; i < latest_usage_.interfaces.size(); ++i) {
                std::cout << "  - " << latest_usage_.interfaces[i] << ": RX=" 
                          << latest_usage_.rx_bytes[i] << " bytes, TX=" 
                          << latest_usage_.tx_bytes[i] << " bytes" << std::endl;
            }
        }

        std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
        std::chrono::milliseconds elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        if (elapsed_time < interval_) {
            std::this_thread::sleep_for(interval_ - elapsed_time);
        }
    }
}

double ResourceMonitor::GetCPUUsage() {
#ifdef __APPLE__
    // macOS implementation using host_statistics
    // This is a simplified version that just estimates CPU load
    host_cpu_load_info_data_t cpu_load;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, 
                        (host_info_t)&cpu_load, &count) != KERN_SUCCESS) {
        return 0.0;
    }
    
    // Calculate total ticks
    uint64_t total_ticks = 0;
    uint64_t idle_ticks = cpu_load.cpu_ticks[CPU_STATE_IDLE];
    
    for (int i = 0; i < CPU_STATE_MAX; i++) {
        total_ticks += cpu_load.cpu_ticks[i];
    }
    
    // Calculate difference from previous values
    uint64_t idle_diff = idle_ticks - prev_idle_time_;
    uint64_t total_diff = total_ticks - prev_total_time_;
    
    // Update previous values
    prev_idle_time_ = idle_ticks;
    prev_total_time_ = total_ticks;
    
    // Calculate CPU usage
    if (total_diff == 0) {
        return 0.0;
    }
    
    return 100.0 * (1.0 - static_cast<double>(idle_diff) / total_diff);
#else
    // Linux implementation
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
#endif
}

size_t ResourceMonitor::GetTotalMemory() {
#ifdef __APPLE__
    // macOS implementation
    int64_t memory = 0;
    size_t length = sizeof(memory);
    if (sysctlbyname("hw.memsize", &memory, &length, NULL, 0) == 0) {
        return static_cast<size_t>(memory);
    }
    return 0;
#else
    // Linux implementation
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) == -1) {
        return 0;
    }
    return sys_info.totalram * sys_info.mem_unit;
#endif
}

size_t ResourceMonitor::GetUsedMemory() {
#ifdef __APPLE__
    // macOS implementation
    vm_size_t page_size;
    mach_port_t mach_port = mach_host_self();
    vm_statistics64_data_t vm_stats;
    mach_msg_type_number_t count = sizeof(vm_stats) / sizeof(natural_t);
    
    if (host_page_size(mach_port, &page_size) != KERN_SUCCESS ||
        host_statistics64(mach_port, HOST_VM_INFO64, 
                        (host_info64_t)&vm_stats, &count) != KERN_SUCCESS) {
        return 0;
    }
    
    // Calculate used memory
    size_t used = (vm_stats.active_count + 
                  vm_stats.inactive_count + 
                  vm_stats.wire_count) * page_size;
    
    return used;
#else
    // Linux implementation
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) == -1) {
        return 0;
    }
    return (sys_info.totalram - sys_info.freeram) * sys_info.mem_unit;
#endif
}

uint64_t ResourceMonitor::GetNetworkBytesReceived(const std::string& interface) {
    #ifdef __APPLE__
        // Mark parameter as used to suppress warnings
        (void)interface;
        
        // macOS implementation - simplified approach
        // Real implementation would parse ifconfig output or use getifaddrs()
        return 1000000;  // Return a dummy value for demo purposes
#else
    // Linux implementation
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
#endif
}

uint64_t ResourceMonitor::GetNetworkBytesSent(const std::string& interface) {
    #ifdef __APPLE__
        // Mark parameter as used to suppress warnings
        (void)interface;
        
        // macOS implementation - simplified approach
        return 500000;  // Return a dummy value for demo purposes
#else
    // Linux implementation
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
#endif
}

uint64_t ResourceMonitor::GetNetworkPacketsReceived(const std::string& interface) {
    #ifdef __APPLE__
        // Mark parameter as used to suppress warnings
        (void)interface;
        
        // macOS implementation - simplified approach
        return 1000;  // Return a dummy value for demo purposes
#else
    // Linux implementation
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
                    uint64_t rx_bytes, rx_packets;
                    if (iss >> rx_bytes >> rx_packets) {
                        net_dev.close();
                        return rx_packets;
                    }
                }
            }
        }
    }
    net_dev.close();
    return 0;
#endif
}

uint64_t ResourceMonitor::GetNetworkPacketsSent(const std::string& interface) {
    #ifdef __APPLE__
        // Mark parameter as used to suppress warnings
        (void)interface;
        
        // macOS implementation - simplified approach
        return 500;  // Return a dummy value for demo purposes
#else
    // Linux implementation
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
                    uint64_t rx_bytes, rx_packets, rx_errs, rx_drop, rx_fifo, rx_frame, rx_comp, rx_mcast;
                    uint64_t tx_bytes, tx_packets;
                    if (iss >> rx_bytes >> rx_packets >> rx_errs >> rx_drop >> rx_fifo >> rx_frame >> rx_comp >> rx_mcast
                            >> tx_bytes >> tx_packets) {
                        net_dev.close();
                        return tx_packets;
                    }
                }
            }
        }
    }
    net_dev.close();
    return 0;
#endif
}

std::vector<std::string> ResourceMonitor::GetNetworkInterfaces() {
    std::vector<std::string> interfaces;
    
#ifdef __APPLE__
    // macOS implementation using getifaddrs()
    struct ifaddrs *ifaddr, *ifa;
    
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return interfaces;
    }
    
    // Find all interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_name != NULL) {
            // Add if not already in the list
            std::string name(ifa->ifa_name);
            if (std::find(interfaces.begin(), interfaces.end(), name) == interfaces.end()) {
                interfaces.push_back(name);
            }
        }
    }
    
    freeifaddrs(ifaddr);
#else
    // Linux implementation
    std::ifstream net_dev("/proc/net/dev");
    if (!net_dev.is_open()) {
        return interfaces;
    }
    std::string line;
    while (std::getline(net_dev, line)) {
        std::istringstream iss(line);
        std::string interface;
        if (iss >> interface) {
            if (interface.back() == ':') {
                interface.pop_back();
                interfaces.push_back(interface);
            }
        }
    }
    net_dev.close();
#endif

    return interfaces;
}

bool ResourceMonitor::IsRunning() const {
    return running_;
}

ResourceUsage ResourceMonitor::GetLatestUsage() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return latest_usage_;
}

void ResourceMonitor::SetInterval(const std::chrono::milliseconds& interval) {
    if (interval.count() == 0) {
        throw std::invalid_argument("Monitoring interval cannot be zero");
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    interval_ = interval;
}

std::chrono::milliseconds ResourceMonitor::GetInterval() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return interval_;
}

void ResourceMonitor::SetCallback(ResourceMonitorCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = callback;
}

}  // namespace bottleneck_analysis
}  // namespace nvmeof