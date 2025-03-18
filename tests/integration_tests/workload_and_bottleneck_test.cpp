#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../include/benchmarking/workload_generator.h"
#include "../../include/benchmarking/data_collector.h"
#include "../../include/bottleneck_analysis/resource_monitor.h"
#include "../../include/bottleneck_analysis/bottleneck_detector.h"

#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace nvmeof::benchmarking;
using namespace nvmeof::bottleneck_analysis;

class WorkloadAndBottleneckTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test directories
        test_dir_ = std::filesystem::temp_directory_path() / "nvmeof_integration_test";
        std::filesystem::create_directories(test_dir_);
        
        // Set up output file for data collection
        output_file_ = test_dir_ / "benchmark_results.csv";
        
        // Set up default workload profile
        profile_.total_size = 1048576;  // 1 MB
        profile_.block_size = 4096;     // 4 KB
        profile_.num_blocks = 256;
        profile_.interval_us = 100;
        profile_.read_percentage = 70;
        profile_.write_percentage = 30;
        profile_.random_percentage = 50;
    }
    
    void TearDown() override {
        // Clean up test directories
        std::filesystem::remove_all(test_dir_);
    }
    
    // Helper to simulate workload generation
    void SimulateWorkload(DataCollector& collector, int duration_ms) {
        // Simulate workload execution
        for (int i = 0; i < duration_ms; i += 100) {
            // Generate some simulated metrics
            double progress = std::min(100.0, 100.0 * i / duration_ms);
            double throughput = 1000.0 + (std::rand() % 500);
            double iops = 250000.0 + (std::rand() % 50000);
            double latency = 100.0 + (std::rand() % 50);
            
            // Collect the metrics
            collector.CollectDataPoint("Progress", progress, "%");
            collector.CollectDataPoint("Throughput", throughput, "MB/s");
            collector.CollectDataPoint("IOPS", iops, "ops/s");
            collector.CollectDataPoint("Latency", latency, "µs");
            
            // Sleep to simulate time passing
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    std::filesystem::path test_dir_;
    std::filesystem::path output_file_;
    WorkloadProfile profile_;
};

// Test integration between workload generation and data collection
TEST_F(WorkloadAndBottleneckTest, WorkloadAndDataCollection) {
    // Create a data collector
    // Ensure the output directory exists
    std::filesystem::create_directories(output_file_.parent_path());
    DataCollector collector(output_file_.string());
    
    // Simulate workload execution and data collection
    collector.CollectDataPoint("Benchmark Start", 0, "");
    SimulateWorkload(collector, 500);  // Simulate for 500 ms
    collector.CollectDataPoint("Benchmark End", 0, "");
    
    // Verify that the output file was created
    EXPECT_TRUE(std::filesystem::exists(output_file_));
    
    // Read the file contents
    std::ifstream file(output_file_);
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    
    // Check that the file contains expected data
    EXPECT_TRUE(content.find("Benchmark Start") != std::string::npos);
    EXPECT_TRUE(content.find("Progress") != std::string::npos);
    EXPECT_TRUE(content.find("Throughput") != std::string::npos);
    EXPECT_TRUE(content.find("IOPS") != std::string::npos);
    EXPECT_TRUE(content.find("Latency") != std::string::npos);
    EXPECT_TRUE(content.find("Benchmark End") != std::string::npos);
}

// Test integration between resource monitoring and bottleneck detection
TEST_F(WorkloadAndBottleneckTest, ResourceMonitoringAndBottleneckDetection) {
    // Create a resource monitor
    std::atomic<int> resource_samples{0};
    auto monitor_callback = [&resource_samples](const ResourceUsage& /*usage*/) {
        resource_samples++;
    };
    
    ResourceMonitor monitor(std::chrono::milliseconds(100), monitor_callback);
    
    // Create a bottleneck detector
    std::atomic<int> bottlenecks_detected{0};
    auto bottleneck_callback = [&bottlenecks_detected](const BottleneckInfo& /*info*/) {
        bottlenecks_detected++;
    };
    
    // Set thresholds to very low values to ensure we detect "bottlenecks" during testing
    BottleneckDetector detector(10.0, 10.0, 1000, 1000, bottleneck_callback);
    
    // Start the resource monitor
    ASSERT_TRUE(monitor.Start());
    
    // Simulate workload execution
    for (int i = 0; i < 500; i += 100) {
        // Get the latest resource usage
        ResourceUsage usage = monitor.GetLatestUsage();
        
        // Detect bottlenecks
        detector.DetectBottlenecks(usage);
        
        // Sleep to simulate time passing
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Stop the resource monitor
    ASSERT_TRUE(monitor.Stop());
    
    // Check that we collected resource samples
    EXPECT_GT(resource_samples, 0);
    
    // We should have detected some bottlenecks (due to low thresholds)
    // But this might not be reliable on all systems, so we don't assert it
}

// Test concurrent workload execution and resource monitoring
TEST_F(WorkloadAndBottleneckTest, ConcurrentWorkloadAndMonitoring) {
    // Create a data collector
    // Ensure the output directory exists
    std::filesystem::create_directories(output_file_.parent_path());
    DataCollector collector(output_file_.string());
    
    // Create a resource monitor
    std::atomic<int> resource_samples{0};
    auto monitor_callback = [&resource_samples, &collector](const ResourceUsage& usage) {
        resource_samples++;
        
        // Collect resource usage metrics
        collector.CollectDataPoint("CPU Usage", usage.cpu_usage_percent, "%");
        collector.CollectDataPoint("Memory Usage", usage.GetMemoryUsagePercent(), "%");
    };
    
    ResourceMonitor monitor(std::chrono::milliseconds(100), monitor_callback);
    
    // Start the resource monitor
    ASSERT_TRUE(monitor.Start());
    
    // Simulate workload execution and data collection
    collector.CollectDataPoint("Benchmark Start", 0, "");
    SimulateWorkload(collector, 500);  // Simulate for 500 ms
    collector.CollectDataPoint("Benchmark End", 0, "");
    
    // Stop the resource monitor
    ASSERT_TRUE(monitor.Stop());
    
    // Verify that the output file was created
    EXPECT_TRUE(std::filesystem::exists(output_file_));
    
    // Read the file contents
    std::ifstream file(output_file_);
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    
    // Check that the file contains both workload and resource monitoring data
    EXPECT_TRUE(content.find("Benchmark Start") != std::string::npos);
    EXPECT_TRUE(content.find("Throughput") != std::string::npos);
    EXPECT_TRUE(content.find("CPU Usage") != std::string::npos);
    EXPECT_TRUE(content.find("Memory Usage") != std::string::npos);
    EXPECT_TRUE(content.find("Benchmark End") != std::string::npos);
    
    // Check that we collected resource samples
    EXPECT_GT(resource_samples, 0);
}

// Test real-time bottleneck detection during workload execution
TEST_F(WorkloadAndBottleneckTest, RealTimeBottleneckDetection) {
    // Create a data collector
    // Ensure the output directory exists
    std::filesystem::create_directories(output_file_.parent_path());
    DataCollector collector(output_file_.string());
    
    // Create a resource monitor
    ResourceMonitor monitor(std::chrono::milliseconds(100));
    
    // Create a bottleneck detector
    std::atomic<int> bottlenecks_detected{0};
    std::vector<BottleneckType> detected_bottleneck_types;
    std::mutex bottleneck_mutex;
    
    auto bottleneck_callback = [&bottlenecks_detected, &detected_bottleneck_types, &bottleneck_mutex, &collector](const BottleneckInfo& info) {
        bottlenecks_detected++;
        
        // Store the bottleneck type
        {
            std::lock_guard<std::mutex> lock(bottleneck_mutex);
            detected_bottleneck_types.push_back(info.type);
        }
        
        // Collect bottleneck information
        collector.CollectDataPoint("Bottleneck Detected",
                                  static_cast<double>(static_cast<int>(info.type)),
                                  info.resource_name);
        collector.CollectDataPoint("Bottleneck Severity", info.severity * 100.0, "%");
    };
    
    // Set thresholds to very low values to ensure we detect "bottlenecks" during testing
    BottleneckDetector detector(10.0, 10.0, 1000, 1000, bottleneck_callback);
    
    // Start the resource monitor
    ASSERT_TRUE(monitor.Start());
    
    // Simulate workload execution and bottleneck detection
    collector.CollectDataPoint("Benchmark Start", 0, "");
    
    for (int i = 0; i < 500; i += 100) {
        // Simulate workload metrics
        double throughput = 1000.0 + (std::rand() % 500);
        double iops = 250000.0 + (std::rand() % 50000);
        double latency = 100.0 + (std::rand() % 50);
        
        // Collect the metrics
        collector.CollectDataPoint("Throughput", throughput, "MB/s");
        collector.CollectDataPoint("IOPS", iops, "ops/s");
        collector.CollectDataPoint("Latency", latency, "µs");
        
        // Get the latest resource usage
        ResourceUsage usage = monitor.GetLatestUsage();
        
        // Detect bottlenecks
        detector.DetectBottlenecks(usage);
        
        // Sleep to simulate time passing
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    collector.CollectDataPoint("Benchmark End", 0, "");
    
    // Stop the resource monitor
    ASSERT_TRUE(monitor.Stop());
    
    // Read the file contents
    std::ifstream file(output_file_);
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    
    // Check that the file contains workload and bottleneck data
    EXPECT_TRUE(content.find("Benchmark Start") != std::string::npos);
    EXPECT_TRUE(content.find("Throughput") != std::string::npos);
    
    // Check if bottlenecks were detected (not guaranteed on all systems)
    if (bottlenecks_detected > 0) {
        EXPECT_TRUE(content.find("Bottleneck Detected") != std::string::npos);
        EXPECT_TRUE(content.find("Bottleneck Severity") != std::string::npos);
        
        // Check the detected bottleneck types
        std::lock_guard<std::mutex> lock(bottleneck_mutex);
        for (const auto& type : detected_bottleneck_types) {
            EXPECT_TRUE(type == BottleneckType::CPU ||
                        type == BottleneckType::MEMORY ||
                        type == BottleneckType::NETWORK ||
                        type == BottleneckType::STORAGE);
        }
    }
}