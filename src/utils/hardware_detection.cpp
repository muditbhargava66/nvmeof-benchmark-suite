#include "utils/hardware_detection.h"
#include "utils/nvmeof_utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <sys/utsname.h>

namespace nvmeof {
namespace utils {

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
    return "";
}

int HardwareDetection::GetCPUCores() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

int HardwareDetection::GetCPUSockets() {
    std::string cpu_info = ReadFileToString("/proc/cpuinfo");
    std::istringstream iss(cpu_info);
    std::string line;
    int sockets = 0;
    while (std::getline(iss, line)) {
        if (line.find("physical id") != std::string::npos) {
            std::vector<std::string> tokens = SplitString(line, ':');
            if (tokens.size() == 2) {
                sockets = std::max(sockets, std::stoi(TrimString(tokens[1])) + 1);
            }
        }
    }
    return sockets;
}

std::vector<std::string> HardwareDetection::GetNVMeDevices() {
    std::vector<std::string> nvme_devices;
    std::string devices_path = "/sys/class/nvme/";
    for (const auto& entry : std::filesystem::directory_iterator(devices_path)) {
        if (entry.is_directory() && entry.path().filename().string().find("nvme") == 0) {
            nvme_devices.push_back(entry.path().filename().string());
        }
    }
    return nvme_devices;
}

}  // namespace utils
}  // namespace nvmeof