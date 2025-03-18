#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../../include/bottleneck_analysis/system_profiler.h"
#include <sstream>
#include <string>
#include <vector>

using namespace nvmeof::bottleneck_analysis;

class SystemProfilerTest : public ::testing::Test {
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

// Test GetOSInfo method
TEST_F(SystemProfilerTest, GetOSInfo) {
    std::string os_info = SystemProfiler::GetOSInfo();
    
    // OS info should not be empty
    EXPECT_FALSE(os_info.empty());
    
    // OS info should contain reasonable strings like "Linux", "Windows", "Darwin", etc.
    // This is a very basic check since we can't predict the exact OS
    EXPECT_TRUE(
        os_info.find("Linux") != std::string::npos ||
        os_info.find("Windows") != std::string::npos ||
        os_info.find("Darwin") != std::string::npos ||
        os_info.find("BSD") != std::string::npos ||
        os_info.find("UNIX") != std::string::npos
    );
}

// Test GetCPUInfo method
TEST_F(SystemProfilerTest, GetCPUInfo) {
    std::string cpu_info = SystemProfiler::GetCPUInfo();
    
    // CPU info should not be empty
    EXPECT_FALSE(cpu_info.empty());
    
    // CPU info should contain reasonable strings like "Intel", "AMD", "CPU", etc.
    // This is a very basic check since we can't predict the exact CPU
    EXPECT_TRUE(
        cpu_info.find("Intel") != std::string::npos ||
        cpu_info.find("AMD") != std::string::npos ||
        cpu_info.find("ARM") != std::string::npos ||
        cpu_info.find("CPU") != std::string::npos ||
        cpu_info.find("Processor") != std::string::npos
    );
}

// Test GetTotalMemory method
TEST_F(SystemProfilerTest, GetTotalMemory) {
    size_t total_memory = SystemProfiler::GetTotalMemory();
    
    // Total memory should be greater than zero
    EXPECT_GT(total_memory, 0UL);
    
    // Total memory should be reasonable (greater than 10 MB)
    // 10 MB = 10 * 1024 * 1024 = 10,485,760 bytes
    EXPECT_GT(total_memory, 10UL * 1024UL * 1024UL);
    
    // Total memory should be reasonable (less than 10 TB)
    // 10 TB = 10 * 1024 * 1024 * 1024 * 1024 = 10,995,116,277,760 bytes
    EXPECT_LT(total_memory, 10UL * 1024UL * 1024UL * 1024UL * 1024UL);
}

// Test GetNetworkInterfaces method
TEST_F(SystemProfilerTest, GetNetworkInterfaces) {
    std::vector<std::string> interfaces = SystemProfiler::GetNetworkInterfaces();
    
    // There should be at least one network interface
    // Note: This might fail in very restricted environments
    EXPECT_FALSE(interfaces.empty());
    
    // Check that none of the interfaces are empty strings
    for (const auto& interface : interfaces) {
        EXPECT_FALSE(interface.empty());
    }
    
    // Common interface names include lo, eth0, en0, wlan0, etc.
    // At least one of these should be present
    bool found_common_interface = false;
    for (const auto& interface : interfaces) {
        if (interface == "lo" || 
            interface.find("eth") == 0 ||
            interface.find("en") == 0 ||
            interface.find("wlan") == 0 ||
            interface.find("docker") == 0 ||
            interface.find("veth") == 0) {
            found_common_interface = true;
            break;
        }
    }
    EXPECT_TRUE(found_common_interface);
}

// Test PrintSystemProfile method
TEST_F(SystemProfilerTest, PrintSystemProfile) {
    // Capture stdout during profile printing
    std::string output = CaptureStdout([]() {
        SystemProfiler::PrintSystemProfile();
    });
    
    // Check that the output contains expected sections
    EXPECT_TRUE(output.find("System Profile") != std::string::npos);
    EXPECT_TRUE(output.find("OS:") != std::string::npos);
    EXPECT_TRUE(output.find("CPU:") != std::string::npos);
    EXPECT_TRUE(output.find("Total Memory:") != std::string::npos);
    EXPECT_TRUE(output.find("Network Interfaces:") != std::string::npos);
}

// Test the formatting of memory size in bytes
TEST_F(SystemProfilerTest, MemorySizeFormatting) {
    // Get total memory
    size_t total_memory = SystemProfiler::GetTotalMemory();
    
    // Capture stdout during profile printing
    std::string output = CaptureStdout([]() {
        SystemProfiler::PrintSystemProfile();
    });
    
    // Convert total memory to string for comparison
    std::string total_memory_str = std::to_string(total_memory);
    
    // Check that the output contains the total memory value
    // Note: This test might be fragile if the formatting changes
    // For example, if memory is formatted as GB instead of bytes
    // If this test fails, consider updating it to match the actual formatting
    EXPECT_TRUE(output.find("Total Memory:") != std::string::npos);
    
    // The string representation might not be exact due to formatting
    // So we only check that some digits of the memory size are present
    if (total_memory_str.length() > 3) {
        // Check first few digits
        EXPECT_TRUE(output.find(total_memory_str.substr(0, 3)) != std::string::npos);
    }
}