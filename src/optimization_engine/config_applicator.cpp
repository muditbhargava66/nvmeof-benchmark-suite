#include "optimization_engine/config_applicator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/sysinfo.h>

namespace nvmeof {
namespace optimization_engine {

void ConfigApplicator::ApplyConfiguration(const std::string& config) {
    std::istringstream iss(config);
    std::string key, value;

    while (std::getline(iss, key, '=') && std::getline(iss, value, ',')) {
        if (key == "cpu_governor") {
            SetCPUGovernor(value);
        } else if (key == "hugepages") {
            EnableHugePages(std::stoul(value));
        } else if (key == "irq_affinity") {
            SetIRQAffinity(value);
        } else if (key == "tcp_rmem") {
            SetTCPRMem(value);
        } else if (key == "tcp_wmem") {
            SetTCPWMem(value);
        } else {
            std::cerr << "Unknown configuration key: " << key << std::endl;
        }
    }
}

void ConfigApplicator::SetCPUGovernor(const std::string& governor) {
    std::vector<std::string> cpu_dirs;
    for (int i = 0; ; ++i) {
        std::string cpu_dir = "/sys/devices/system/cpu/cpu" + std::to_string(i) + "/cpufreq/scaling_governor";
        if (access(cpu_dir.c_str(), F_OK) != 0) {
            break;
        }
        cpu_dirs.push_back(cpu_dir);
    }

    for (const auto& cpu_dir : cpu_dirs) {
        std::ofstream file(cpu_dir);
        if (file.is_open()) {
            file << governor;
            file.close();
        } else {
            std::cerr << "Failed to set CPU governor for " << cpu_dir << std::endl;
        }
    }
}

void ConfigApplicator::EnableHugePages(size_t num_pages) {
    std::string hugepages_file = "/proc/sys/vm/nr_hugepages";
    std::ofstream file(hugepages_file);
    if (file.is_open()) {
        file << num_pages;
        file.close();
    } else {
        std::cerr << "Failed to enable huge pages" << std::endl;
    }
}

void ConfigApplicator::SetIRQAffinity(const std::string& irq_affinity) {
    std::string irq_affinity_file = "/proc/irq/default_smp_affinity";
    std::ofstream file(irq_affinity_file);
    if (file.is_open()) {
        file << irq_affinity;
        file.close();
    } else {
        std::cerr << "Failed to set IRQ affinity" << std::endl;
    }
}

void ConfigApplicator::SetTCPRMem(const std::string& rmem_values) {
    std::string tcp_rmem_file = "/proc/sys/net/ipv4/tcp_rmem";
    std::ofstream file(tcp_rmem_file);
    if (file.is_open()) {
        file << rmem_values;
        file.close();
    } else {
        std::cerr << "Failed to set TCP receive buffer sizes" << std::endl;
    }
}

void ConfigApplicator::SetTCPWMem(const std::string& wmem_values) {
    std::string tcp_wmem_file = "/proc/sys/net/ipv4/tcp_wmem";
    std::ofstream file(tcp_wmem_file);
    if (file.is_open()) {
        file << wmem_values;
        file.close();
    } else {
        std::cerr << "Failed to set TCP send buffer sizes" << std::endl;
    }
}

}  // namespace optimization_engine
}  // namespace nvmeof