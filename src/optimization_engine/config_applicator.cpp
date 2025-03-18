#include "../../include/optimization_engine/config_applicator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <algorithm>  // for std::replace

// Platform-specific includes
#ifdef __APPLE__
#include <sys/types.h>
#include <sys/sysctl.h>
#else
#include <sys/sysinfo.h>
#endif

namespace nvmeof {
namespace optimization_engine {

void ConfigApplicator::ApplyConfiguration(const std::string& config) {
    if (config.empty()) {
        return; // Nothing to do with empty config
    }
    
    std::vector<std::string> config_items;
    std::istringstream iss(config);
    std::string item;
    
    // Split the config by commas
    while (std::getline(iss, item, ',')) {
        config_items.push_back(item);
    }
    
    // Process each config item
    for (const auto& item : config_items) {
        // Find the equal sign to separate key and value
        size_t pos = item.find('=');
        if (pos == std::string::npos) {
            std::cerr << "Malformed configuration item: " << item << std::endl;
            continue;
        }
        
        std::string key = item.substr(0, pos);
        std::string value = item.substr(pos + 1);
        
        try {
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
            } else if (key == "net.core.rmem_max") {
                SetSysctlValue("kern.ipc.maxsockbuf", value);
            } else {
#ifdef __APPLE__
                // On macOS, just log the issue instead of treating it as an error
                std::cout << "Unknown configuration key (ignored on macOS): " << key << std::endl;
#else
                std::cerr << "Unknown configuration key: " << key << std::endl;
#endif
            }
        } catch (const std::exception& e) {
            std::cerr << "Error applying configuration for key '" << key << "': " << e.what() << std::endl;
        }
    }
}

void ConfigApplicator::SetCPUGovernor(const std::string& governor) {
#ifdef __APPLE__
    // Mark parameter as used to suppress warning
    (void)governor;

    // macOS doesn't support CPU governors like Linux
    std::cerr << "Setting CPU governor is not supported on macOS" << std::endl;
#else
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
#endif
}

void ConfigApplicator::EnableHugePages(size_t num_pages) {
#ifdef __APPLE__
    // Mark parameter as used to suppress warning
    (void)num_pages;

    // macOS doesn't support huge pages in the same way as Linux
    std::cerr << "Setting huge pages is not supported on macOS" << std::endl;
#else
    std::string hugepages_file = "/proc/sys/vm/nr_hugepages";
    std::ofstream file(hugepages_file);
    if (file.is_open()) {
        file << num_pages;
        file.close();
    } else {
        std::cerr << "Failed to enable huge pages" << std::endl;
    }
#endif
}

void ConfigApplicator::SetIRQAffinity(const std::string& irq_affinity) {
#ifdef __APPLE__
    // Mark parameter as used to suppress warning
    (void)irq_affinity;

    // macOS handles IRQ affinity differently
    std::cerr << "Setting IRQ affinity is not supported on macOS" << std::endl;
#else
    std::string irq_affinity_file = "/proc/irq/default_smp_affinity";
    std::ofstream file(irq_affinity_file);
    if (file.is_open()) {
        file << irq_affinity;
        file.close();
    } else {
        std::cerr << "Failed to set IRQ affinity" << std::endl;
    }
#endif
}

void ConfigApplicator::SetTCPRMem(const std::string& rmem_values) {
#ifdef __APPLE__
    // Mark parameter as used to suppress warning
    (void)rmem_values;
    
    // macOS uses sysctl for network tuning rather than procfs
    std::cerr << "Setting TCP receive buffer sizes is not supported on macOS" << std::endl;
#else
    std::string tcp_rmem_file = "/proc/sys/net/ipv4/tcp_rmem";
    std::ofstream file(tcp_rmem_file);
    if (file.is_open()) {
        file << rmem_values;
        file.close();
    } else {
        std::cerr << "Failed to set TCP receive buffer sizes" << std::endl;
    }
#endif
}

void ConfigApplicator::SetTCPWMem(const std::string& wmem_values) {
#ifdef __APPLE__
    // Mark parameter as used to suppress warning
    (void)wmem_values;

    // macOS uses sysctl for network tuning rather than procfs
    std::cerr << "Setting TCP send buffer sizes is not supported on macOS" << std::endl;
#else
    std::string tcp_wmem_file = "/proc/sys/net/ipv4/tcp_wmem";
    std::ofstream file(tcp_wmem_file);
    if (file.is_open()) {
        file << wmem_values;
        file.close();
    } else {
        std::cerr << "Failed to set TCP send buffer sizes" << std::endl;
    }
#endif
}

void ConfigApplicator::SetSysctlValue(const std::string& key, const std::string& value) {
#ifdef __APPLE__
    // macOS implementation using sysctlbyname
    std::cout << "Setting sysctl " << key << " to " << value << " (simulation on macOS)" << std::endl;
    // In a real macOS implementation, you would use sysctlbyname here
#else
    // Linux implementation
    std::string sysctl_path = "/proc/sys/" + key;
    std::replace(sysctl_path.begin(), sysctl_path.end(), '.', '/');
    
    std::ofstream file(sysctl_path);
    if (file.is_open()) {
        file << value;
        file.close();
    } else {
        std::cerr << "Failed to set sysctl " << key << std::endl;
    }
#endif
}

}  // namespace optimization_engine
}  // namespace nvmeof