#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../../include/optimization_engine/optimizer.h"
#include "../../../include/optimization_engine/config_knowledge_base.h"
#include "../../../include/optimization_engine/config_applicator.h"
#include "../../../include/bottleneck_analysis/bottleneck_detector.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>

using namespace nvmeof::optimization_engine;
using namespace nvmeof::bottleneck_analysis;

// Mock ConfigKnowledgeBase
class MockConfigKnowledgeBase : public ConfigKnowledgeBase {
public:
    MockConfigKnowledgeBase() : ConfigKnowledgeBase("") {}
    
    MOCK_METHOD(std::string, GetConfigValue, (const std::string& key), (const));
};

// Mock BottleneckDetector
class MockBottleneckDetector : public BottleneckDetector {
public:
    MockBottleneckDetector() : BottleneckDetector() {}
    
    MOCK_METHOD(std::vector<BottleneckInfo>, DetectBottlenecks, 
               (double cpu_usage, double memory_usage, uint64_t network_usage, uint64_t storage_usage), (const, override));
};

// Mock ConfigApplicator
class MockConfigApplicator : public ConfigApplicator {
public:
    MOCK_METHOD(void, ApplyConfiguration, (const std::string& config));
};

class OptimizerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create mock objects
        mock_config_kb_ = std::make_unique<MockConfigKnowledgeBase>();
        mock_bottleneck_detector_ = std::make_unique<MockBottleneckDetector>();
        
        // Set up default CPU, memory, and network usage values
        cpu_usage_ = 50.0;
        memory_usage_ = 40.0;
        network_usage_ = 500000000; // 500 MB/s
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
    
    std::unique_ptr<MockConfigKnowledgeBase> mock_config_kb_;
    std::unique_ptr<MockBottleneckDetector> mock_bottleneck_detector_;
    double cpu_usage_;
    double memory_usage_;
    uint64_t network_usage_;
};

// Test constructor
TEST_F(OptimizerTest, Constructor) {
    Optimizer optimizer(*mock_config_kb_, *mock_bottleneck_detector_);
    
    // No assertions needed; just ensure it doesn't crash
}

// Test OptimizeConfiguration with no bottlenecks
TEST_F(OptimizerTest, OptimizeConfigurationNoBottlenecks) {
    // Set up mock behavior - empty bottlenecks list
    std::vector<BottleneckInfo> empty_bottlenecks;
    
    // Set up the mock to return empty bottlenecks
    EXPECT_CALL(*mock_bottleneck_detector_, DetectBottlenecks(cpu_usage_, memory_usage_, network_usage_, 0))
        .WillOnce(testing::Return(empty_bottlenecks));
    
    // Create optimizer
    Optimizer optimizer(*mock_config_kb_, *mock_bottleneck_detector_);
    
    // Capture stdout during optimization
    std::string output = CaptureStdout([&]() {
        optimizer.OptimizeConfiguration(cpu_usage_, memory_usage_, network_usage_);
    });
    
    // No bottlenecks, so expect no calls to GetConfigValue and no output
    // In the Optimizer implementation, if bottlenecks is empty it falls back to GetBottleneckType,
    // but since our test uses values below the thresholds, it should output nothing.
    EXPECT_TRUE(output.find("No bottlenecks detected") != std::string::npos);
}

// Test OptimizeConfiguration with CPU bottleneck
TEST_F(OptimizerTest, OptimizeConfigurationCpuBottleneck) {
    // Set up mock behavior for bottleneck detection
    std::vector<BottleneckInfo> bottlenecks = {
        BottleneckInfo(
            BottleneckType::CPU,
            "High CPU usage detected",
            0.5,
            "CPU",
            90.0,
            "Optimize CPU settings"
        )
    };
    
    EXPECT_CALL(*mock_bottleneck_detector_, DetectBottlenecks(90.0, memory_usage_, network_usage_, 0))
        .WillOnce(testing::Return(bottlenecks));
    
    // Set up mock behavior for config knowledge base
    EXPECT_CALL(*mock_config_kb_, GetConfigValue("cpu_bottleneck"))
        .WillOnce(testing::Return("cpu_governor=performance"));
    
    // Create optimizer
    Optimizer optimizer(*mock_config_kb_, *mock_bottleneck_detector_);
    
    // Capture stdout during optimization
    std::string output = CaptureStdout([&]() {
        optimizer.OptimizeConfiguration(90.0, memory_usage_, network_usage_);
    });
    
    // Check that the output contains the optimization config
    EXPECT_TRUE(output.find("Bottleneck detected") != std::string::npos);
    EXPECT_TRUE(output.find("Optimization config for cpu_bottleneck") != std::string::npos);
    EXPECT_TRUE(output.find("cpu_governor=performance") != std::string::npos);
}

// Test OptimizeConfiguration with memory bottleneck
TEST_F(OptimizerTest, OptimizeConfigurationMemoryBottleneck) {
    // Set up mock behavior for bottleneck detection
    std::vector<BottleneckInfo> bottlenecks = {
        BottleneckInfo(
            BottleneckType::MEMORY,
            "High memory usage detected",
            0.7,
            "Memory",
            95.0,
            "Optimize memory settings"
        )
    };
    
    EXPECT_CALL(*mock_bottleneck_detector_, DetectBottlenecks(cpu_usage_, 95.0, network_usage_, 0))
        .WillOnce(testing::Return(bottlenecks));
    
    // Set up mock behavior for config knowledge base
    EXPECT_CALL(*mock_config_kb_, GetConfigValue("memory_bottleneck"))
        .WillOnce(testing::Return("vm.swappiness=10"));
    
    // Create optimizer
    Optimizer optimizer(*mock_config_kb_, *mock_bottleneck_detector_);
    
    // Capture stdout during optimization
    std::string output = CaptureStdout([&]() {
        optimizer.OptimizeConfiguration(cpu_usage_, 95.0, network_usage_);
    });
    
    // Check that the output contains the optimization config
    EXPECT_TRUE(output.find("Bottleneck detected") != std::string::npos);
    EXPECT_TRUE(output.find("Optimization config for memory_bottleneck") != std::string::npos);
    EXPECT_TRUE(output.find("vm.swappiness=10") != std::string::npos);
}

// Test OptimizeConfiguration with network bottleneck
TEST_F(OptimizerTest, OptimizeConfigurationNetworkBottleneck) {
    // Set up mock behavior for bottleneck detection
    std::vector<BottleneckInfo> bottlenecks = {
        BottleneckInfo(
            BottleneckType::NETWORK,
            "High network usage detected",
            0.6,
            "Network",
            1500000000.0,
            "Optimize network settings"
        )
    };
    
    EXPECT_CALL(*mock_bottleneck_detector_, DetectBottlenecks(cpu_usage_, memory_usage_, 1500000000, 0))
        .WillOnce(testing::Return(bottlenecks));
    
    // Set up mock behavior for config knowledge base
    EXPECT_CALL(*mock_config_kb_, GetConfigValue("network_bottleneck"))
        .WillOnce(testing::Return("net.core.rmem_max=16777216"));
    
    // Create optimizer
    Optimizer optimizer(*mock_config_kb_, *mock_bottleneck_detector_);
    
    // Capture stdout during optimization
    std::string output = CaptureStdout([&]() {
        optimizer.OptimizeConfiguration(cpu_usage_, memory_usage_, 1500000000);
    });
    
    // Check that the output contains the optimization config
    EXPECT_TRUE(output.find("Bottleneck detected") != std::string::npos);
    EXPECT_TRUE(output.find("Optimization config for network_bottleneck") != std::string::npos);
    EXPECT_TRUE(output.find("net.core.rmem_max=16777216") != std::string::npos);
}

// Test OptimizeConfiguration with multiple bottlenecks
TEST_F(OptimizerTest, OptimizeConfigurationMultipleBottlenecks) {
    // Set up mock behavior for bottleneck detection
    std::vector<BottleneckInfo> bottlenecks = {
        BottleneckInfo(
            BottleneckType::CPU,
            "High CPU usage detected",
            0.5,
            "CPU",
            90.0,
            "Optimize CPU settings"
        ),
        BottleneckInfo(
            BottleneckType::MEMORY,
            "High memory usage detected",
            0.7,
            "Memory",
            95.0,
            "Optimize memory settings"
        )
    };
    
    EXPECT_CALL(*mock_bottleneck_detector_, DetectBottlenecks(90.0, 95.0, network_usage_, 0))
        .WillOnce(testing::Return(bottlenecks));
    
    // Set up mock behavior for config knowledge base
    EXPECT_CALL(*mock_config_kb_, GetConfigValue("cpu_bottleneck"))
        .WillOnce(testing::Return("cpu_governor=performance"));
    
    EXPECT_CALL(*mock_config_kb_, GetConfigValue("memory_bottleneck"))
        .WillOnce(testing::Return("vm.swappiness=10"));
    
    // Create optimizer
    Optimizer optimizer(*mock_config_kb_, *mock_bottleneck_detector_);
    
    // Capture stdout during optimization
    std::string output = CaptureStdout([&]() {
        optimizer.OptimizeConfiguration(90.0, 95.0, network_usage_);
    });
    
    // Check that the output contains both optimization configs
    EXPECT_TRUE(output.find("Bottleneck detected") != std::string::npos);
    EXPECT_TRUE(output.find("Optimization config for cpu_bottleneck") != std::string::npos);
    EXPECT_TRUE(output.find("cpu_governor=performance") != std::string::npos);
    EXPECT_TRUE(output.find("Optimization config for memory_bottleneck") != std::string::npos);
    EXPECT_TRUE(output.find("vm.swappiness=10") != std::string::npos);
}

// Test OptimizeConfiguration with bottleneck but no config
TEST_F(OptimizerTest, OptimizeConfigurationBottleneckNoConfig) {
    // Set up mock behavior for bottleneck detection
    std::vector<BottleneckInfo> bottlenecks = {
        BottleneckInfo(
            BottleneckType::CPU,
            "High CPU usage detected",
            0.5,
            "CPU",
            90.0,
            "Optimize CPU settings"
        )
    };
    
    EXPECT_CALL(*mock_bottleneck_detector_, DetectBottlenecks(90.0, memory_usage_, network_usage_, 0))
        .WillOnce(testing::Return(bottlenecks));
    
    // Set up mock behavior for config knowledge base (empty config)
    EXPECT_CALL(*mock_config_kb_, GetConfigValue("cpu_bottleneck"))
        .WillOnce(testing::Return(""));
    
    // Create optimizer
    Optimizer optimizer(*mock_config_kb_, *mock_bottleneck_detector_);
    
    // Capture stdout during optimization
    std::string output = CaptureStdout([&]() {
        optimizer.OptimizeConfiguration(90.0, memory_usage_, network_usage_);
    });
    
    // Should only show bottleneck detected, but no optimization config
    EXPECT_TRUE(output.find("Bottleneck detected") != std::string::npos);
    EXPECT_TRUE(output.find("Optimization config") == std::string::npos);
}

/*
// Test with real ConfigApplicator - NOTE: THIS TEST WILL FAIL
// This test demonstrates a limitation in the current design where we can't
// inject a mock ConfigApplicator into the Optimizer
TEST_F(OptimizerTest, DISABLED_OptimizeConfigurationWithMockApplicator) {
    // Set up mock behavior for bottleneck detection
    std::vector<BottleneckInfo> bottlenecks = {
        BottleneckInfo(
            BottleneckType::CPU,
            "High CPU usage detected",
            0.5,
            "CPU",
            90.0,
            "Optimize CPU settings"
        )
    };
    
    EXPECT_CALL(*mock_bottleneck_detector_, DetectBottlenecks(90.0, memory_usage_, network_usage_, 0))
        .WillOnce(testing::Return(bottlenecks));
    
    // Set up mock behavior for config knowledge base
    EXPECT_CALL(*mock_config_kb_, GetConfigValue("cpu_bottleneck"))
        .WillOnce(testing::Return("cpu_governor=performance"));
    
    // Create a mock ConfigApplicator
    MockConfigApplicator mock_applicator;
    
    // This test won't work as intended because the mock_applicator isn't used by the optimizer.
    // In a future design, we should modify the Optimizer to accept a ConfigApplicator
    // through dependency injection. For now, we're disabling this test.
    
    // The following expectations won't be met because the mock isn't injected:
    EXPECT_CALL(mock_applicator, ApplyConfiguration("cpu_governor=performance"))
        .Times(0); // Changed to 0 since we know it won't be called
    
    // Create optimizer with the bottleneck detector and config knowledge base
    Optimizer optimizer(*mock_config_kb_, *mock_bottleneck_detector_);
    
    // Note: We can't inject the mock_applicator directly since Optimizer doesn't have a setter for it
    // This is a limitation of the current design
    
    // Capture stdout and errors
    std::string output = CaptureStdout([&]() {
        testing::internal::CaptureStderr();
        
        // Optimize configuration
        optimizer.OptimizeConfiguration(90.0, memory_usage_, network_usage_);
        
        testing::internal::GetCapturedStderr();
    });
    
    // The test would pass if the mock applicator's ApplyConfiguration was called with the right argument
    // but currently this isn't possible without modifying the Optimizer class to accept a ConfigApplicator
}
    */