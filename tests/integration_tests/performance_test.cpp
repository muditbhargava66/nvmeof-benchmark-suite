#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../include/benchmarking/workload_generator.h"
#include "../../include/benchmarking/data_collector.h"
#include "../../include/benchmarking/result_visualizer.h"
#include "../../include/bottleneck_analysis/resource_monitor.h"
#include "../../include/bottleneck_analysis/bottleneck_detector.h"
#include "../../include/optimization_engine/config_knowledge_base.h"
#include "../../include/optimization_engine/optimizer.h"
#include "../../include/utils/nvmeof_utils.h"

#include <chrono>
#include <thread>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace nvmeof::benchmarking;
using namespace nvmeof::bottleneck_analysis;
using namespace nvmeof::optimization_engine;
using namespace nvmeof::utils;

class PerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directories
        test_dir_ = std::filesystem::temp_directory_path() / "nvmeof_perf_test";
        std::filesystem::create_directories(test_dir_);
        results_dir_ = test_dir_ / "results";
        std::filesystem::create_directories(results_dir_);
        
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
    
    // Helper to measure execution time
    template<typename Func>
    std::chrono::microseconds MeasureExecutionTime(Func func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    }
    
    // Helper to simulate workload
    void SimulateWorkload(DataCollector& collector, int operations) {
        for (int i = 0; i < operations; ++i) {
            // Generate some simulated metrics
            double progress = 100.0 * i / operations;
            double throughput = 1000.0 + (std::rand() % 500);
            double iops = 250000.0 + (std::rand() % 50000);
            double latency = 100.0 + (std::rand() % 50);
            
            // Collect the metrics
            collector.CollectDataPoint("Progress", progress, "%");
            collector.CollectDataPoint("Throughput", throughput, "MB/s");
            collector.CollectDataPoint("IOPS", iops, "ops/s");
            collector.CollectDataPoint("Latency", latency, "µs");
        }
    }
    
    std::filesystem::path test_dir_;
    std::filesystem::path results_dir_;
    WorkloadProfile profile_;
};

// Test data collection performance
TEST_F(PerformanceTest, DataCollectionPerformance) {
    // Create a data collector
    std::filesystem::path output_file = results_dir_ / "data_collection_perf.csv";
    // Ensure the output directory exists
    std::filesystem::create_directories(output_file.parent_path());
    DataCollector collector(output_file.string());
    
    // Measure the time to collect 1000 data points
    const int num_points = 1000;
    auto execution_time = MeasureExecutionTime([&]() {
        SimulateWorkload(collector, num_points);
    });
    
    // Calculate data points per second
    double points_per_second = static_cast<double>(num_points) / 
                              (execution_time.count() / 1000000.0);
    
    // Log performance statistics
    std::cout << "Data Collection Performance:" << std::endl;
    std::cout << "  Total time: " << execution_time.count() / 1000.0 << " ms" << std::endl;
    std::cout << "  Data points: " << num_points << std::endl;
    std::cout << "  Data points per second: " << points_per_second << std::endl;
    
    // Check that the performance is reasonable
    // This is subjective and depends on the system, but we can set a minimum threshold
    EXPECT_GT(points_per_second, 1000.0);  // At least 1000 points per second
    
    // Check that the data was actually collected
    EXPECT_TRUE(std::filesystem::exists(output_file));
    EXPECT_EQ(num_points, collector.GetDataPointCount());
}

// Test resource monitoring performance
TEST_F(PerformanceTest, ResourceMonitoringPerformance) {
    // Create a resource monitor
    std::vector<ResourceUsage> collected_samples;
    auto callback = [&collected_samples](const ResourceUsage& usage) {
        collected_samples.push_back(usage);
    };
    
    ResourceMonitor monitor(std::chrono::milliseconds(10), callback);
    
    // Start the monitor and collect samples for a specific duration
    const int duration_ms = 200;
    ASSERT_TRUE(monitor.Start());
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    ASSERT_TRUE(monitor.Stop());
    
    // Calculate samples per second
    double samples_per_second = static_cast<double>(collected_samples.size()) / 
                               (duration_ms / 1000.0);
    
    // Log performance statistics
    std::cout << "Resource Monitoring Performance:" << std::endl;
    std::cout << "  Duration: " << duration_ms << " ms" << std::endl;
    std::cout << "  Samples collected: " << collected_samples.size() << std::endl;
    std::cout << "  Samples per second: " << samples_per_second << std::endl;
    
    // Check that the performance is reasonable
    // We set the interval to 10ms, so we should get approximately 100 samples per second
    // Allow for some variation due to scheduling
    EXPECT_GE(samples_per_second, 50.0);   // At least 50 samples per second
    EXPECT_LE(samples_per_second, 150.0);  // At most 150 samples per second
}

// Test bottleneck detection performance
TEST_F(PerformanceTest, BottleneckDetectionPerformance) {
    // Create a bottleneck detector
    BottleneckDetector detector;
    
    // Measure the time to perform bottleneck detection for a large number of samples
    const int num_samples = 1000;
    std::vector<std::chrono::microseconds> detection_times;
    
    for (int i = 0; i < num_samples; ++i) {
        // Generate random resource usage values
        double cpu_usage = std::rand() % 101;  // 0-100%
        double memory_usage = std::rand() % 101;  // 0-100%
        uint64_t network_usage = std::rand() % 2000000000;  // 0-2GB/s
        uint64_t storage_usage = std::rand() % 1000000000;  // 0-1GB/s
        
        // Measure the time to detect bottlenecks
        auto detection_time = MeasureExecutionTime([&]() {
            detector.DetectBottlenecks(cpu_usage, memory_usage, network_usage, storage_usage);
        });
        
        detection_times.push_back(detection_time);
    }
    
    // Calculate statistics
    double total_ms = std::accumulate(detection_times.begin(), detection_times.end(), 0.0,
                                     [](double sum, const auto& time) {
                                         return sum + time.count() / 1000.0;
                                     });
    double avg_ms = total_ms / num_samples;
    
    auto min_time = *std::min_element(detection_times.begin(), detection_times.end());
    auto max_time = *std::max_element(detection_times.begin(), detection_times.end());
    
    // Log performance statistics
    std::cout << "Bottleneck Detection Performance:" << std::endl;
    std::cout << "  Total time: " << total_ms << " ms" << std::endl;
    std::cout << "  Number of samples: " << num_samples << std::endl;
    std::cout << "  Average time per detection: " << avg_ms << " ms" << std::endl;
    std::cout << "  Min time: " << min_time.count() / 1000.0 << " ms" << std::endl;
    std::cout << "  Max time: " << max_time.count() / 1000.0 << " ms" << std::endl;
    
    // Check that the performance is reasonable
    // Bottleneck detection should be very fast (sub-millisecond)
    EXPECT_LT(avg_ms, 1.0);  // Average time should be less than 1 ms
}

// Test optimization performance
TEST_F(PerformanceTest, OptimizationPerformance) {
    // Create a config knowledge base
    std::filesystem::path config_file = test_dir_ / "test_config.ini";
    std::ofstream file(config_file);
    file << "cpu_bottleneck=cpu_governor=performance\n";
    file << "memory_bottleneck=vm.swappiness=10\n";
    file << "network_bottleneck=net.core.rmem_max=16777216\n";
    file.close();
    
    ConfigKnowledgeBase kb(config_file.string());
    
    // Create a bottleneck detector
    BottleneckDetector detector;
    
    // Create an optimizer
    Optimizer optimizer(kb, detector);
    
    // Measure the time to perform optimization for a large number of samples
    const int num_samples = 1000;
    std::vector<std::chrono::microseconds> optimization_times;
    
    for (int i = 0; i < num_samples; ++i) {
        // Generate random resource usage values
        double cpu_usage = std::rand() % 101;  // 0-100%
        double memory_usage = std::rand() % 101;  // 0-100%
        uint64_t network_usage = std::rand() % 2000000000;  // 0-2GB/s
        
        // Measure the time to optimize
        auto optimization_time = MeasureExecutionTime([&]() {
            // Redirect stdout to suppress output
            std::stringstream ss;
            std::streambuf* old = std::cout.rdbuf(ss.rdbuf());
            
            optimizer.OptimizeConfiguration(cpu_usage, memory_usage, network_usage);
            
            // Restore stdout
            std::cout.rdbuf(old);
        });
        
        optimization_times.push_back(optimization_time);
    }
    
    // Calculate statistics
    double total_ms = std::accumulate(optimization_times.begin(), optimization_times.end(), 0.0,
                                     [](double sum, const auto& time) {
                                         return sum + time.count() / 1000.0;
                                     });
    double avg_ms = total_ms / num_samples;
    
    auto min_time = *std::min_element(optimization_times.begin(), optimization_times.end());
    auto max_time = *std::max_element(optimization_times.begin(), optimization_times.end());
    
    // Log performance statistics
    std::cout << "Optimization Performance:" << std::endl;
    std::cout << "  Total time: " << total_ms << " ms" << std::endl;
    std::cout << "  Number of samples: " << num_samples << std::endl;
    std::cout << "  Average time per optimization: " << avg_ms << " ms" << std::endl;
    std::cout << "  Min time: " << min_time.count() / 1000.0 << " ms" << std::endl;
    std::cout << "  Max time: " << max_time.count() / 1000.0 << " ms" << std::endl;
    
    // Check that the performance is reasonable
    // Optimization should be relatively fast (single-digit milliseconds)
    EXPECT_LT(avg_ms, 10.0);  // Average time should be less than 10 ms
}

// Test file I/O performance
TEST_F(PerformanceTest, FileIOPerformance) {
    // Measure write performance
    const int file_size_mb = 10;
    const int chunk_size_kb = 64;
    std::filesystem::path test_file = test_dir_ / "io_test.bin";
    
    std::string chunk(chunk_size_kb * 1024, 'A');  // 64KB chunk of data
    
    // Measure write time
    auto write_time = MeasureExecutionTime([&]() {
        std::ofstream file(test_file, std::ios::binary);
        ASSERT_TRUE(file.is_open());
        
        for (int i = 0; i < file_size_mb * 1024 / chunk_size_kb; ++i) {
            file.write(chunk.data(), chunk.size());
        }
        
        file.close();
    });
    
    // Measure read time
    std::string read_buffer(chunk_size_kb * 1024, '\0');
    auto read_time = MeasureExecutionTime([&]() {
        std::ifstream file(test_file, std::ios::binary);
        ASSERT_TRUE(file.is_open());
        
        while (file.read(&read_buffer[0], read_buffer.size())) {
            // Just read the data, don't do anything with it
        }
        
        file.close();
    });
    
    // Calculate throughput
    double write_throughput_mbps = (file_size_mb * 1024 * 1024) / 
                                  (write_time.count() / 1000000.0) / 
                                  (1024 * 1024);
    
    double read_throughput_mbps = (file_size_mb * 1024 * 1024) / 
                                 (read_time.count() / 1000000.0) / 
                                 (1024 * 1024);
    
    // Log performance statistics
    std::cout << "File I/O Performance:" << std::endl;
    std::cout << "  File size: " << file_size_mb << " MB" << std::endl;
    std::cout << "  Chunk size: " << chunk_size_kb << " KB" << std::endl;
    std::cout << "  Write time: " << write_time.count() / 1000.0 << " ms" << std::endl;
    std::cout << "  Read time: " << read_time.count() / 1000.0 << " ms" << std::endl;
    std::cout << "  Write throughput: " << write_throughput_mbps << " MB/s" << std::endl;
    std::cout << "  Read throughput: " << read_throughput_mbps << " MB/s" << std::endl;
    
    // Check that the performance is reasonable
    // This is highly dependent on the system, but we can set minimum thresholds
    EXPECT_GT(write_throughput_mbps, 10.0);  // At least 10 MB/s write
    EXPECT_GT(read_throughput_mbps, 10.0);   // At least 10 MB/s read
}

// Test end-to-end performance
TEST_F(PerformanceTest, EndToEndPerformance) {
    // Create components
    std::filesystem::path output_file = results_dir_ / "end_to_end_perf.csv";
    // Ensure the output directory exists
    std::filesystem::create_directories(output_file.parent_path());
    DataCollector collector(output_file.string());
    
    ResourceMonitor monitor(std::chrono::milliseconds(100));
    
    BottleneckDetector detector;
    
    std::filesystem::path config_file = test_dir_ / "test_config.ini";
    std::ofstream file(config_file);
    file << "cpu_bottleneck=cpu_governor=performance\n";
    file << "memory_bottleneck=vm.swappiness=10\n";
    file << "network_bottleneck=net.core.rmem_max=16777216\n";
    file.close();
    
    ConfigKnowledgeBase kb(config_file.string());
    
    Optimizer optimizer(kb, detector);
    
    // Start the resource monitor
    ASSERT_TRUE(monitor.Start());
    
    // Measure end-to-end performance
    const int num_operations = 100;
    auto execution_time = MeasureExecutionTime([&]() {
        collector.CollectDataPoint("Benchmark Start", 0, "");
        
        for (int i = 0; i < num_operations; ++i) {
            // Simulate workload metrics
            double progress = 100.0 * i / num_operations;
            double throughput = 1000.0 + (std::rand() % 500);
            double iops = 250000.0 + (std::rand() % 50000);
            double latency = 100.0 + (std::rand() % 50);
            
            // Collect the metrics
            collector.CollectDataPoint("Progress", progress, "%");
            collector.CollectDataPoint("Throughput", throughput, "MB/s");
            collector.CollectDataPoint("IOPS", iops, "ops/s");
            collector.CollectDataPoint("Latency", latency, "µs");
            
            // Get the latest resource usage
            ResourceUsage usage = monitor.GetLatestUsage();
            
            // Perform bottleneck detection and optimization
            std::stringstream ss;
            std::streambuf* old = std::cout.rdbuf(ss.rdbuf());
            
            optimizer.OptimizeConfiguration(
                usage.cpu_usage_percent,
                usage.GetMemoryUsagePercent(),
                usage.rx_bytes.empty() ? 0 : usage.rx_bytes[0]
            );
            
            std::cout.rdbuf(old);
        }
        
        collector.CollectDataPoint("Benchmark End", 0, "");
    });
    
    // Stop the resource monitor
    ASSERT_TRUE(monitor.Stop());
    
    // Calculate operations per second
    double ops_per_second = static_cast<double>(num_operations) / 
                           (execution_time.count() / 1000000.0);
    
    // Log performance statistics
    std::cout << "End-to-End Performance:" << std::endl;
    std::cout << "  Total time: " << execution_time.count() / 1000.0 << " ms" << std::endl;
    std::cout << "  Number of operations: " << num_operations << std::endl;
    std::cout << "  Operations per second: " << ops_per_second << std::endl;
    std::cout << "  Average time per operation: " << execution_time.count() / num_operations / 1000.0 << " ms" << std::endl;
    
    // Check that the data was actually collected
    EXPECT_TRUE(std::filesystem::exists(output_file));
}