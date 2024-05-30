#pragma once

#include <string>
#include <vector>

namespace nvmeof {
namespace bottleneck_analysis {

class SystemProfiler {
public:
    static std::string GetOSInfo();
    static std::string GetCPUInfo();
    static size_t GetTotalMemory();
    static std::vector<std::string> GetNetworkInterfaces();
    static void PrintSystemProfile();
};

}  // namespace bottleneck_analysis
}  // namespace nvmeof