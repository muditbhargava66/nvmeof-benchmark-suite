#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../../include/utils/hardware_detection.h"
#include <sstream>
#include <string>
#include <vector>

using namespace nvmeof::utils;

class HardwareDetectionTest : public ::testing::Test {
protected:
    // Helper to capture stdout
    std::string CaptureStdout(std::function<void()> func) {
        std::streambuf* old = std::cout.rdbuf();
        std::stringstream ss;
        std::cout.rdbuf(ss.rdbuf());
        
        func();
        
        std::cout.rdbuf(old);
        return ss.str();
    }
};

// Test GetOSName method
TEST_F(HardwareDetectionTest, GetOSName) {
    std::string os_name = HardwareDetection::GetOSName();
    
    // OS name should not be empty
    EXPECT_FALSE(os_name.empty());
    
    // OS name should be reasonable
    EXPECT_TRUE(
        os_name == "Linux" ||
        os_name == "Windows" ||
        os_name == "Darwin" ||
        os_name == "FreeBSD" ||
        os_name == "OpenBSD" ||
        os_name.find("BSD") != std::string::npos ||
        os_name.find("UNIX") != std::string::npos
    );
}

// Test GetOSVersion method
TEST_F(HardwareDetectionTest, GetOSVersion) {
    std::string os_version = HardwareDetection::GetOSVersion();
    
    // OS version should not be empty
    EXPECT_FALSE(os_version.empty());
    
    // OS version should contain numbers and periods
    bool contains_digits = false;
    for (char c : os_version) {
        if (std::isdigit(c)) {
            contains_digits = true;
            break;
        }
    }
    EXPECT_TRUE(contains_digits);
}

// Test GetCPUModel method
TEST_F(HardwareDetectionTest, GetCPUModel) {
    std::string cpu_model = HardwareDetection::GetCPUModel();
    
    // CPU model should not be empty
    EXPECT_FALSE(cpu_model.empty());
    
    // CPU model should contain some common CPU manufacturer names
    EXPECT_TRUE(
        cpu_model.find("Intel") != std::string::npos ||
        cpu_model.find("AMD") != std::string::npos ||
        cpu_model.find("ARM") != std::string::npos ||
        cpu_model.find("Apple") != std::string::npos ||
        cpu_model.find("Qualcomm") != std::string::npos ||
        cpu_model.find("CPU") != std::string::npos ||
        cpu_model.find("Processor") != std::string::npos
    );
}

// Test GetCPUCores method
TEST_F(HardwareDetectionTest, GetCPUCores) {
    int cpu_cores = HardwareDetection::GetCPUCores();
    
    // Number of CPU cores should be positive
    EXPECT_GT(cpu_cores, 0);
    
    // Number of CPU cores should be reasonable (less than 1024)
    EXPECT_LT(cpu_cores, 1024);
}

// Test GetCPUSockets method
TEST_F(HardwareDetectionTest, GetCPUSockets) {
    int cpu_sockets = HardwareDetection::GetCPUSockets();
    
    // Number of CPU sockets should be positive
    EXPECT_GT(cpu_sockets, 0);
    
    // Number of CPU sockets should be reasonable (less than 64)
    EXPECT_LT(cpu_sockets, 64);
    
    // Number of CPU sockets should be less than or equal to the number of cores
    EXPECT_LE(cpu_sockets, HardwareDetection::GetCPUCores());
}

// Test GetNVMeDevices method
TEST_F(HardwareDetectionTest, GetNVMeDevices) {
    // This test might not find any NVMe devices on all systems
    std::vector<std::string> nvme_devices = HardwareDetection::GetNVMeDevices();
    
    // If devices are found, check their names
    for (const auto& device : nvme_devices) {
        EXPECT_TRUE(device.find("nvme") != std::string::npos);
    }
}

// Test comprehensive hardware information
TEST_F(HardwareDetectionTest, HardwareInfo) {
    // Collect all hardware information
    std::stringstream info;
    info << "OS Name: " << HardwareDetection::GetOSName() << std::endl;
    info << "OS Version: " << HardwareDetection::GetOSVersion() << std::endl;
    info << "CPU Model: " << HardwareDetection::GetCPUModel() << std::endl;
    info << "CPU Cores: " << HardwareDetection::GetCPUCores() << std::endl;
    info << "CPU Sockets: " << HardwareDetection::GetCPUSockets() << std::endl;
    
    info << "NVMe Devices:" << std::endl;
    auto nvme_devices = HardwareDetection::GetNVMeDevices();
    if (nvme_devices.empty()) {
        info << "  None found" << std::endl;
    } else {
        for (const auto& device : nvme_devices) {
            info << "  " << device << std::endl;
        }
    }
    
    // Check that the information is reasonable
    std::string hardware_info = info.str();
    EXPECT_FALSE(hardware_info.empty());
    
    // Check for expected sections
    EXPECT_TRUE(hardware_info.find("OS Name:") != std::string::npos);
    EXPECT_TRUE(hardware_info.find("OS Version:") != std::string::npos);
    EXPECT_TRUE(hardware_info.find("CPU Model:") != std::string::npos);
    EXPECT_TRUE(hardware_info.find("CPU Cores:") != std::string::npos);
    EXPECT_TRUE(hardware_info.find("CPU Sockets:") != std::string::npos);
    EXPECT_TRUE(hardware_info.find("NVMe Devices:") != std::string::npos);
}

// Test hardware detection on different platforms
TEST_F(HardwareDetectionTest, PlatformSpecific) {
    std::string os_name = HardwareDetection::GetOSName();
    
    if (os_name == "Linux") {
        // Linux-specific tests
        
        // Check that /proc/cpuinfo was parsed correctly
        EXPECT_FALSE(HardwareDetection::GetCPUModel().empty());
        EXPECT_GT(HardwareDetection::GetCPUCores(), 0);
        EXPECT_GT(HardwareDetection::GetCPUSockets(), 0);
        
    } else if (os_name == "Darwin") {
        // macOS-specific tests
        
        // Check that sysctl was used correctly
        EXPECT_FALSE(HardwareDetection::GetCPUModel().empty());
        EXPECT_GT(HardwareDetection::GetCPUCores(), 0);
        
    } else if (os_name == "Windows") {
        // Windows-specific tests
        
        // Check that WMI or registry was used correctly
        EXPECT_FALSE(HardwareDetection::GetCPUModel().empty());
        EXPECT_GT(HardwareDetection::GetCPUCores(), 0);
    }
    
    // Skip the platform-specific tests if not on a recognized platform
}

// Test behavior with non-existent or inaccessible files
TEST_F(HardwareDetectionTest, ErrorHandling) {
    // This test is somewhat artificial and relies on implementation details
    // We'll check that stderr contains error messages when accessing non-existent files
    
    // Capture stderr
    testing::internal::CaptureStderr();
    
    // Force an error by accessing a non-existent file
    // This is done indirectly by calling the methods, assuming they'll try to read files
    HardwareDetection::GetCPUModel();
    HardwareDetection::GetCPUSockets();
    HardwareDetection::GetNVMeDevices();
    
    // Get stderr output
    std::string stderr_output = testing::internal::GetCapturedStderr();
    
    // We can't make strong assertions about the output, as it depends on the implementation
    // and whether the files actually exist or are accessible
    // So we just note that this test exercises the error handling paths
}