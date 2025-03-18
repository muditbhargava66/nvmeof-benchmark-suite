#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

#include "../../include/benchmarking/workload_generator.h"
#include "../../include/benchmarking/data_collector.h"
#include "../../include/benchmarking/result_visualizer.h"
#include "../../include/bottleneck_analysis/resource_monitor.h"
#include "../../include/bottleneck_analysis/bottleneck_detector.h"
#include "../../include/optimization_engine/config_knowledge_base.h"
#include "../../include/optimization_engine/optimizer.h"
#include "../../include/utils/nvmeof_utils.h"

namespace fs = std::filesystem;

namespace {
    // Mock NVMe controller for testing
    struct MockNvmeCtrlr {
        // Mock implementation
    };

    // Mock NVMe queue pair for testing
    struct MockNvmeQpair {
        // Mock implementation
    };
}

// Helper function to create test directory
bool createTestDirectory(const std::string& dir_path) {
    try {
        fs::create_directories(dir_path);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create directory: " << e.what() << std::endl;
        return false;
    }
}

// Helper function to clean up test directory
bool cleanupTestDirectory(const std::string& dir_path) {
    try {
        fs::remove_all(dir_path);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to cleanup directory: " << e.what() << std::endl;
        return false;
    }
}

// Test fixture for end-to-end tests
class EndToEndTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directories
        test_dir_ = "./test_output/end_to_end_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        createTestDirectory(test_dir_);
        createTestDirectory(test_dir_ + "/results");
        
        // Set up workload profile
        profile_.total_size = 1048576;        // 1 MB
        profile_.block_size = 4096;           // 4 KB
        profile_.num_blocks = 256;            // 256 blocks
        profile_.interval_us = 100;           // 100 microseconds
        profile_.read_percentage = 70;        // 70% reads
        profile_.write_percentage = 30;       // 30% writes
        profile_.random_percentage = 50;      // 50% random
    }
    
    void TearDown() override {
        // Clean up test directories
        cleanupTestDirectory(test_dir_);
    }
    
    // Create a mock workload generation completion callback
    nvmeof::benchmarking::IoCompletionCallback createMockCallback() {
        return [this](bool success, uint32_t bytes_processed) {
            callback_called_ = true;
            callback_success_ = success;
            callback_bytes_ = bytes_processed;
        };
    }
    
    std::string test_dir_;
    nvmeof::benchmarking::WorkloadProfile profile_;
    
    // Callback tracking variables
    bool callback_called_ = false;
    bool callback_success_ = false;
    uint32_t callback_bytes_ = 0;
};

// End-to-end test for the full workflow
TEST_F(EndToEndTest, DISABLED_FullWorkflow) {
    // Since we can't test with real NVMe hardware, we'll use mock objects
    // DISABLED_ prefix prevents the test from running automatically
    
    // This is a placeholder for a real end-to-end test that would:
    // 1. Generate workloads
    // 2. Monitor resources
    // 3. Detect bottlenecks
    // 4. Apply optimizations
    // 5. Visualize results
    
    // In a real test, you would:
    
    // 1. Set up the data collector
    std::string results_file = test_dir_ + "/results/benchmark_results.csv";
    nvmeof::benchmarking::DataCollector collector(results_file);
    
    // 2. Create and start the resource monitor
    nvmeof::bottleneck_analysis::ResourceMonitor monitor(std::chrono::milliseconds(100));
    monitor.Start();
    
    // 3. Set up the bottleneck detector
    nvmeof::bottleneck_analysis::BottleneckDetector detector(80.0, 90.0, 1000000000);
    
    // 4. Load configuration knowledge base
    std::string config_file = "path/to/test/config.ini";
    // Create a sample config file for testing
    std::ofstream config_out(config_file);
    config_out << "cpu_bottleneck=cpu_governor=performance\n";
    config_out << "memory_bottleneck=hugepages=1024\n";
    config_out.close();
    
    nvmeof::optimization_engine::ConfigKnowledgeBase kb(config_file);
    
    // 5. Create optimizer
    nvmeof::optimization_engine::Optimizer optimizer(kb, detector);
    
    // 6. Run the test workflow
    
    // Since we can't run with real hardware, we'll just check that the components
    // can be created without errors
    
    EXPECT_TRUE(fs::exists(results_file));
    EXPECT_TRUE(monitor.IsRunning());
    
    // Cleanup
    monitor.Stop();
    fs::remove(config_file);
}

// Test for data collection and visualization flow
TEST_F(EndToEndTest, DataCollectionAndVisualization) {
    // Set up the data collector
    std::string results_file = test_dir_ + "/results/benchmark_results.csv";
    nvmeof::benchmarking::DataCollector collector(results_file);
    
    // Create directory if it doesn't exist
    std::filesystem::create_directories(std::filesystem::path(results_file).parent_path());
    
    // Collect some test data points
    ASSERT_TRUE(collector.CollectDataPoint("Throughput", 1200.5, "MB/s"));
    ASSERT_TRUE(collector.CollectDataPoint("IOPS", 250000, "ops/s"));
    ASSERT_TRUE(collector.CollectDataPoint("Latency", 120, "µs"));
    
    // Ensure the file was created
    ASSERT_TRUE(fs::exists(results_file));
    
    // Set up the visualizer
    nvmeof::benchmarking::ResultVisualizer visualizer(results_file);
    
    // Visualize the results (this just prints to console in our implementation)
    visualizer.Visualize();
    
    // In a real test, we would check that the visualization was created correctly
    // For now, we'll just check that the file exists and has content
    std::ifstream results_in(results_file);
    std::string content((std::istreambuf_iterator<char>(results_in)),
                        std::istreambuf_iterator<char>());
    
    EXPECT_FALSE(content.empty());
    EXPECT_TRUE(content.find("Throughput") != std::string::npos);
    EXPECT_TRUE(content.find("IOPS") != std::string::npos);
    EXPECT_TRUE(content.find("Latency") != std::string::npos);
}

// Test for resource monitoring and bottleneck detection flow
TEST_F(EndToEndTest, ResourceMonitoringAndBottleneckDetection) {
    // Set up the resource monitor with a callback
    bool callback_called = false;
    nvmeof::bottleneck_analysis::ResourceUsage captured_usage;
    
    auto monitor_callback = [&callback_called, &captured_usage](const nvmeof::bottleneck_analysis::ResourceUsage& usage) {
        callback_called = true;
        captured_usage = usage;
    };
    
    nvmeof::bottleneck_analysis::ResourceMonitor monitor(std::chrono::milliseconds(100), monitor_callback);
    
    // Set up the bottleneck detector with a callback
    bool bottleneck_detected = false;
    nvmeof::bottleneck_analysis::BottleneckInfo detected_bottleneck(
        nvmeof::bottleneck_analysis::BottleneckType::NONE, "", 0.0, "", 0.0, "");
    
    auto bottleneck_callback = [&bottleneck_detected, &detected_bottleneck](const nvmeof::bottleneck_analysis::BottleneckInfo& info) {
        bottleneck_detected = true;
        detected_bottleneck = info;
    };
    
    nvmeof::bottleneck_analysis::BottleneckDetector detector(80.0, 90.0, 1000000000, 500000000, bottleneck_callback);
    
    // Start monitoring
    ASSERT_TRUE(monitor.Start());
    
    // Give it some time to collect data
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    // Get the latest resource usage
    auto usage = monitor.GetLatestUsage();
    
    // Simulate a CPU bottleneck
    detector.DetectBottlenecks(90.0, 70.0, 500000000, 100000000);
    
    // Stop monitoring
    ASSERT_TRUE(monitor.Stop());
    
    // Check that we got some resource usage data
    EXPECT_TRUE(callback_called);
    
    // Check that we detected a bottleneck
    EXPECT_TRUE(bottleneck_detected);
    EXPECT_EQ(nvmeof::bottleneck_analysis::BottleneckType::CPU, detected_bottleneck.type);
}

// More integration tests would be added here to test different workflows and component interactions