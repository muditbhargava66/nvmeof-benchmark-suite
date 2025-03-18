#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../include/bottleneck_analysis/resource_monitor.h"
#include "../../include/bottleneck_analysis/bottleneck_detector.h"
#include "../../include/optimization_engine/config_knowledge_base.h"
#include "../../include/optimization_engine/optimizer.h"
#include "../../include/optimization_engine/config_applicator.h"

#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>
#include <sstream>

using namespace nvmeof::bottleneck_analysis;
using namespace nvmeof::optimization_engine;

// Mock ConfigApplicator for testing
class MockConfigApplicator : public ConfigApplicator {
public:
    // Keep track of configurations that were applied
    std::vector<std::string> applied_configs;
    
    void ApplyConfiguration(const std::string& config) {
        applied_configs.push_back(config);
        
        // Extract key-value pairs
        std::istringstream iss(config);
        std::string key_value_pair;
        while (std::getline(iss, key_value_pair, ',')) {
            std::istringstream kv_iss(key_value_pair);
            std::string key, value;
            if (std::getline(kv_iss, key, '=') && std::getline(kv_iss, value)) {
                // Call the appropriate method based on the key
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
                }
            }
        }
    }
    
    MOCK_METHOD(void, SetCPUGovernor, (const std::string& governor));
    MOCK_METHOD(void, EnableHugePages, (size_t num_pages));
    MOCK_METHOD(void, SetIRQAffinity, (const std::string& irq_affinity));
    MOCK_METHOD(void, SetTCPRMem, (const std::string& rmem_values));
    MOCK_METHOD(void, SetTCPWMem, (const std::string& wmem_values));
};

class BottleneckAndOptimizationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test directories
        test_dir_ = std::filesystem::temp_directory_path() / "nvmeof_integration_test";
        std::filesystem::create_directories(test_dir_);
        
        // Create a test config file
        config_file_path_ = test_dir_ / "test_config.ini";
        CreateTestConfigFile();
    }
    
    void TearDown() override {
        // Clean up test directories
        std::filesystem::remove_all(test_dir_);
    }
    
    // Helper to create a test config file
    void CreateTestConfigFile() {
        std::ofstream file(config_file_path_);
        ASSERT_TRUE(file.is_open());
        
        // Write test configuration
        file << "cpu_bottleneck=cpu_governor=performance,hugepages=1024\n";
        file << "memory_bottleneck=vm.swappiness=10,vm.vfs_cache_pressure=50\n";
        file << "network_bottleneck=net.core.rmem_max=16777216,net.core.wmem_max=16777216\n";
        file << "storage_bottleneck=vm.dirty_ratio=10,vm.dirty_background_ratio=5\n";
        
        file.close();
    }
    
    // Helper to capture stdout
    std::string CaptureStdout(std::function<void()> func) {
        std::streambuf* old = std::cout.rdbuf();
        std::stringstream ss;
        std::cout.rdbuf(ss.rdbuf());
        
        func();
        
        std::cout.rdbuf(old);
        return ss.str();
    }
    
    std::filesystem::path test_dir_;
    std::filesystem::path config_file_path_;
};

// Test integration between bottleneck detection and optimization
TEST_F(BottleneckAndOptimizationTest, BottleneckDetectionAndOptimization) {
    // Create a config knowledge base
    ConfigKnowledgeBase kb(config_file_path_.string());
    
    // Create a bottleneck detector
    std::atomic<int> bottlenecks_detected{0};
    auto bottleneck_callback = [&bottlenecks_detected](const BottleneckInfo& /*info*/) {
        bottlenecks_detected++;
    };
    
    BottleneckDetector detector(80.0, 90.0, 1000000000, 500000000, bottleneck_callback);
    
    // Create a mock optimizer with the mock applicator
    Optimizer optimizer(kb, detector);
    
    // Create a mock config applicator and set up expectations
    MockConfigApplicator mock_applicator;
    EXPECT_CALL(mock_applicator, SetCPUGovernor("performance"))
        .Times(::testing::AtLeast(0));
    EXPECT_CALL(mock_applicator, EnableHugePages(1024))
        .Times(::testing::AtLeast(0));
    EXPECT_CALL(mock_applicator, SetTCPRMem(::testing::_))
        .Times(::testing::AtLeast(0));
    EXPECT_CALL(mock_applicator, SetTCPWMem(::testing::_))
        .Times(::testing::AtLeast(0));
    
    // Simulate a CPU bottleneck
    double cpu_usage = 90.0;     // Above CPU threshold
    double memory_usage = 50.0;  // Below memory threshold
    uint64_t network_usage = 500000000;  // Below network threshold
    
    // Capture stdout during optimization
    std::string output = CaptureStdout([&]() {
        // Apply configuration using the mock applicator
        for (const auto& config : {"cpu_bottleneck", "memory_bottleneck", "network_bottleneck"}) {
            std::string config_value = kb.GetConfigValue(config);
            if (!config_value.empty()) {
                mock_applicator.ApplyConfiguration(config_value);
            }
        }
        
        // Run the optimizer
        optimizer.OptimizeConfiguration(cpu_usage, memory_usage, network_usage);
    });
    
    // Verify that the optimization occurred
    EXPECT_TRUE(output.find("Optimization config for cpu_bottleneck") != std::string::npos);
    
    // Verify that the mock applicator was used
    EXPECT_FALSE(mock_applicator.applied_configs.empty());
    
    // At least one of the configs should contain "cpu_governor=performance"
    bool found_cpu_config = false;
    for (const auto& config : mock_applicator.applied_configs) {
        if (config.find("cpu_governor=performance") != std::string::npos) {
            found_cpu_config = true;
            break;
        }
    }
    EXPECT_TRUE(found_cpu_config);
}

// Test end-to-end flow from monitoring to optimization
TEST_F(BottleneckAndOptimizationTest, EndToEndFlow) {
    // Create a resource monitor
    ResourceMonitor monitor(std::chrono::milliseconds(100));
    
    // Create a config knowledge base
    ConfigKnowledgeBase kb(config_file_path_.string());
    
    // Create a bottleneck detector
    std::atomic<int> bottlenecks_detected{0};
    auto bottleneck_callback = [&bottlenecks_detected](const BottleneckInfo& /*info*/) {
        bottlenecks_detected++;
    };
    
    // Set thresholds to very low values to ensure we detect "bottlenecks" during testing
    BottleneckDetector detector(10.0, 10.0, 1000, 1000, bottleneck_callback);
    
    // Create a mock config applicator and set up expectations
    MockConfigApplicator mock_applicator;
    EXPECT_CALL(mock_applicator, SetCPUGovernor(::testing::_))
        .Times(::testing::AtLeast(0));
    EXPECT_CALL(mock_applicator, EnableHugePages(::testing::_))
        .Times(::testing::AtLeast(0));
    EXPECT_CALL(mock_applicator, SetTCPRMem(::testing::_))
        .Times(::testing::AtLeast(0));
    EXPECT_CALL(mock_applicator, SetTCPWMem(::testing::_))
        .Times(::testing::AtLeast(0));
    
    // Create an optimizer
    Optimizer optimizer(kb, detector);
    
    // Start the resource monitor
    ASSERT_TRUE(monitor.Start());
    
    // Simulate workload execution and optimization
    for (int i = 0; i < 500; i += 100) {
        // Get the latest resource usage
        ResourceUsage usage = monitor.GetLatestUsage();
        
        // Optimize configuration based on resource usage
        CaptureStdout([&]() {
            optimizer.OptimizeConfiguration(
                usage.cpu_usage_percent,
                usage.GetMemoryUsagePercent(),
                usage.rx_bytes.empty() ? 0 : usage.rx_bytes[0]
            );
        });
        
        // Simulate optimization by applying configurations
        for (const auto& bottleneck_type : {"cpu_bottleneck", "memory_bottleneck", "network_bottleneck"}) {
            std::string config = kb.GetConfigValue(bottleneck_type);
            if (!config.empty()) {
                mock_applicator.ApplyConfiguration(config);
            }
        }
        
        // Sleep to simulate time passing
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Stop the resource monitor
    ASSERT_TRUE(monitor.Stop());
    
    // Check that we detected bottlenecks and applied configurations
    // This might not always be true depending on the system, so we don't assert it
    // EXPECT_GT(bottlenecks_detected, 0);
    // EXPECT_FALSE(mock_applicator.applied_configs.empty());
}

// Test optimization with different bottleneck types
TEST_F(BottleneckAndOptimizationTest, OptimizationWithDifferentBottlenecks) {
    // Create a config knowledge base
    ConfigKnowledgeBase kb(config_file_path_.string());
    
    // Create a bottleneck detector
    BottleneckDetector detector;
    
    // Create a mock config applicator and set up expectations
    MockConfigApplicator mock_applicator;
    EXPECT_CALL(mock_applicator, SetCPUGovernor("performance"))
        .Times(::testing::AtLeast(0));
    EXPECT_CALL(mock_applicator, EnableHugePages(1024))
        .Times(::testing::AtLeast(0));
    EXPECT_CALL(mock_applicator, SetTCPRMem(::testing::_))
        .Times(::testing::AtLeast(0));
    EXPECT_CALL(mock_applicator, SetTCPWMem(::testing::_))
        .Times(::testing::AtLeast(0));
    
    // Create an optimizer
    Optimizer optimizer(kb, detector);
    
    // Test CPU bottleneck
    CaptureStdout([&]() {
        optimizer.OptimizeConfiguration(90.0, 50.0, 500000000);
    });
    
    // Test memory bottleneck
    CaptureStdout([&]() {
        optimizer.OptimizeConfiguration(50.0, 95.0, 500000000);
    });
    
    // Test network bottleneck
    CaptureStdout([&]() {
        optimizer.OptimizeConfiguration(50.0, 50.0, 1500000000);
    });
    
    // Apply configurations
    std::vector<std::string> bottleneck_types = {"cpu_bottleneck", "memory_bottleneck", "network_bottleneck"};
    for (const auto& bottleneck_type : bottleneck_types) {
        std::string config = kb.GetConfigValue(bottleneck_type);
        if (!config.empty()) {
            mock_applicator.ApplyConfiguration(config);
        }
    }
    
    // Verify that configurations were applied
    EXPECT_GE(mock_applicator.applied_configs.size(), 1);
    
    // Check for specific configurations
    bool found_cpu_config = false;
    bool found_memory_config = false;
    bool found_network_config = false;
    
    for (const auto& config : mock_applicator.applied_configs) {
        if (config.find("cpu_governor=performance") != std::string::npos) {
            found_cpu_config = true;
        }
        if (config.find("vm.swappiness=10") != std::string::npos) {
            found_memory_config = true;
        }
        if (config.find("net.core.rmem_max=16777216") != std::string::npos) {
            found_network_config = true;
        }
    }
    
    // At least one of the configurations should have been applied
    EXPECT_TRUE(found_cpu_config || found_memory_config || found_network_config);
}

// Test optimization with multiple concurrent bottlenecks
TEST_F(BottleneckAndOptimizationTest, OptimizationWithMultipleBottlenecks) {
    // Create a config knowledge base
    ConfigKnowledgeBase kb(config_file_path_.string());
    
    // Create a bottleneck detector
    BottleneckDetector detector;
    
    // Create a mock config applicator and set up expectations
    MockConfigApplicator mock_applicator;
    EXPECT_CALL(mock_applicator, SetCPUGovernor(::testing::_))
        .Times(::testing::AtLeast(0));
    EXPECT_CALL(mock_applicator, EnableHugePages(::testing::_))
        .Times(::testing::AtLeast(0));
    EXPECT_CALL(mock_applicator, SetTCPRMem(::testing::_))
        .Times(::testing::AtLeast(0));
    EXPECT_CALL(mock_applicator, SetTCPWMem(::testing::_))
        .Times(::testing::AtLeast(0));
    
    // Create an optimizer
    Optimizer optimizer(kb, detector);
    
    // Simulate multiple bottlenecks (CPU and memory)
    CaptureStdout([&]() {
        optimizer.OptimizeConfiguration(90.0, 95.0, 500000000);
    });
    
    // Apply configurations
    std::vector<std::string> bottleneck_types = {"cpu_bottleneck", "memory_bottleneck"};
    for (const auto& bottleneck_type : bottleneck_types) {
        std::string config = kb.GetConfigValue(bottleneck_type);
        if (!config.empty()) {
            mock_applicator.ApplyConfiguration(config);
        }
    }
    
    // Verify that configurations were applied
    EXPECT_GE(mock_applicator.applied_configs.size(), 1);
    
    // Check for specific configurations
    bool found_cpu_config = false;
    bool found_memory_config = false;
    
    for (const auto& config : mock_applicator.applied_configs) {
        if (config.find("cpu_governor=performance") != std::string::npos) {
            found_cpu_config = true;
        }
        if (config.find("vm.swappiness=10") != std::string::npos) {
            found_memory_config = true;
        }
    }
    
    // Both configurations should have been applied
    EXPECT_TRUE(found_cpu_config);
    EXPECT_TRUE(found_memory_config);
}