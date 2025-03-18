#include "../../include/utils/hardware_detection.h"
#include "../../include/utils/nvmeof_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/utsname.h>

// Include platform-specific headers
#ifdef __APPLE__
#include <sys/sysctl.h>
#include <sys/types.h>
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

namespace nvmeof {
namespace utils {

// Helper function to get sysctl value on macOS
#ifdef __APPLE__
std::string GetSysctlString(const std::string& name) {
    char buffer[256];
    size_t size = sizeof(buffer);
    if (sysctlbyname(name.c_str(), buffer, &size, nullptr, 0) == 0) {
        return std::string(buffer, size);
    }
    return "";
}

int GetSysctlInt(const std::string& name) {
    int value;
    size_t size = sizeof(value);
    if (sysctlbyname(name.c_str(), &value, &size, nullptr, 0) == 0) {
        return value;
    }
    return 0;
}
#endif

std::string HardwareDetection::GetOSName() {
    struct utsname uname_data;
    if (uname(&uname_data) != 0) {
        std::cerr << "Failed to retrieve OS name" << std::endl;
        return "";
    }
    return uname_data.sysname;
}

std::string HardwareDetection::GetOSVersion() {
    struct utsname uname_data;
    if (uname(&uname_data) != 0) {
        std::cerr << "Failed to retrieve OS version" << std::endl;
        return "";
    }
    return uname_data.release;
}

std::string HardwareDetection::GetCPUModel() {
#ifdef __APPLE__
    // macOS-specific implementation
    return GetSysctlString("machdep.cpu.brand_string");
#else
    // Linux implementation
    std::string cpu_info = ReadFileToString("/proc/cpuinfo");
    std::istringstream iss(cpu_info);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find("model name") != std::string::npos) {
            std::vector<std::string> tokens = SplitString(line, ':');
            if (tokens.size() == 2) {
                return TrimString(tokens[1]);
            }
        }
    }
#endif
    return "Unknown CPU";
}

int HardwareDetection::GetCPUCores() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

int HardwareDetection::GetCPUSockets() {
#ifdef __APPLE__
    // macOS doesn't have a direct way to get socket count, assume 1 for desktop/laptop
    return 1;
#else
    std::string cpu_info = ReadFileToString("/proc/cpuinfo");
    std::istringstream iss(cpu_info);
    std::string line;
    std::vector<int> physical_ids;
    
    while (std::getline(iss, line)) {
        if (line.find("physical id") != std::string::npos) {
            std::vector<std::string> tokens = SplitString(line, ':');
            if (tokens.size() == 2) {
                int physical_id = std::stoi(TrimString(tokens[1]));
                if (std::find(physical_ids.begin(), physical_ids.end(), physical_id) == physical_ids.end()) {
                    physical_ids.push_back(physical_id);
                }
            }
        }
    }
    return physical_ids.empty() ? 1 : physical_ids.size();
#endif
}

std::vector<std::string> HardwareDetection::GetNVMeDevices() {
    std::vector<std::string> nvme_devices;
    
#ifdef __APPLE__
    // macOS: simulate presence of devices for development
    nvme_devices.push_back("nvme0");
    nvme_devices.push_back("nvme1");
#else
    // Linux implementation
    std::string devices_path = "/sys/class/nvme/";
    if (fs::exists(devices_path)) {
        for (const auto& entry : fs::directory_iterator(devices_path)) {
            if (entry.is_directory() && entry.path().filename().string().find("nvme") == 0) {
                nvme_devices.push_back(entry.path().filename().string());
            }
        }
    }
#endif
    
    return nvme_devices;
}

}  // namespace utils
}  // namespace nvmeof