#include "bottleneck_analysis/system_profiler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>

namespace nvmeof {
namespace bottleneck_analysis {

std::string SystemProfiler::GetOSInfo() {
    struct utsname uname_data;
    if (uname(&uname_data) == -1) {
        return "Unknown";
    }
    return uname_data.sysname + std::string(" ") + uname_data.release;
}

std::string SystemProfiler::GetCPUInfo() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (!cpuinfo.is_open()) {
        return "Unknown";
    }
    std::string line;
    std::string cpu_model;
    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos) {
            cpu_model = line.substr(line.find(":") + 2);
            break;
        }
    }
    cpuinfo.close();
    return cpu_model;
}

size_t SystemProfiler::GetTotalMemory() {
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) == -1) {
        return 0;
    }
    return sys_info.totalram * sys_info.mem_unit;
}

std::vector<std::string> SystemProfiler::GetNetworkInterfaces() {
    std::vector<std::string> interfaces;
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
    return interfaces;
}

void SystemProfiler::PrintSystemProfile() {
    std::cout << "System Profile:" << std::endl;
    std::cout << "OS: " << GetOSInfo() << std::endl;
    std::cout << "CPU: " << GetCPUInfo() << std::endl;
    std::cout << "Total Memory: " << GetTotalMemory() << " bytes" << std::endl;
    std::cout << "Network Interfaces:" << std::endl;
    for (const auto& interface : GetNetworkInterfaces()) {
        std::cout << "  - " << interface << std::endl;
    }
}

}  // namespace bottleneck_analysis
}  // namespace nvmeof