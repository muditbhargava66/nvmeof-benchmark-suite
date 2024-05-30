#pragma once

#include <string>
#include <vector>

namespace nvmeof {
namespace utils {

class HardwareDetection {
public:
    static std::string GetOSName();
    static std::string GetOSVersion();
    static std::string GetCPUModel();
    static int GetCPUCores();
    static int GetCPUSockets();
    static std::vector<std::string> GetNVMeDevices();
};

}  // namespace utils
}  // namespace nvmeof