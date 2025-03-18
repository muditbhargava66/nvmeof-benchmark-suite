#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../../include/benchmarking/result_visualizer.h"
#include "../../../include/benchmarking/data_collector.h"
#include <fstream>
#include <string>
#include <filesystem>
#include <sstream>

using namespace nvmeof::benchmarking;

class ResultVisualizerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test directory
        test_dir_ = std::filesystem::temp_directory_path() / "nvmeof_test";
        std::filesystem::create_directories(test_dir_);
        
        // Set up test file paths
        input_file_path_ = test_dir_ / "test_input.csv";
        
        // Create a test input file with sample data
        CreateTestInputFile();
    }
    
    void TearDown() override {
        // Clean up test files and directory
        std::filesystem::remove_all(test_dir_);
    }
    
    // Helper to create a test input file
    void CreateTestInputFile() {
        std::ofstream file(input_file_path_);
        ASSERT_TRUE(file.is_open());
        
        // Write CSV header
        file << "Timestamp,Label,Value,Units\n";
        
        // Write sample data points
        file << "2023-01-01 12:00:00,Throughput,1024.5,MB/s\n";
        file << "2023-01-01 12:00:05,IOPS,250000,ops/s\n";
        file << "2023-01-01 12:00:10,Latency,100.2,µs\n";
        file << "2023-01-01 12:00:15,CPU Usage,45.3,%\n";
        file << "2023-01-01 12:00:20,Memory Usage,32.8,%\n";
        
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
    std::filesystem::path input_file_path_;
};

// Test constructor with non-existent file
TEST_F(ResultVisualizerTest, ConstructorNonExistentFile) {
    // Non-existent file should not throw, but should log an error
    std::filesystem::path non_existent_file = test_dir_ / "non_existent.csv";
    
    // Capture stderr to check for error message
    testing::internal::CaptureStderr();
    
    // Create a visualizer with a non-existent file
    ResultVisualizer visualizer(non_existent_file.string());
    
    // Get stderr output
    std::string stderr_output = testing::internal::GetCapturedStderr();
    
    // Check for error message
    EXPECT_TRUE(stderr_output.find("Error opening file") != std::string::npos);
}

// Test Visualize method
TEST_F(ResultVisualizerTest, Visualize) {
    // Create a visualizer with the test input file
    ResultVisualizer visualizer(input_file_path_.string());
    
    // Capture stdout during visualization
    std::string output = CaptureStdout([&visualizer]() {
        visualizer.Visualize();
    });
    
    // Check that the output contains expected information
    EXPECT_TRUE(output.find("Benchmark Results") != std::string::npos);
    EXPECT_TRUE(output.find("Timestamp") != std::string::npos);
    EXPECT_TRUE(output.find("Data Point") != std::string::npos);
    EXPECT_TRUE(output.find("Throughput") != std::string::npos);
    EXPECT_TRUE(output.find("IOPS") != std::string::npos);
    EXPECT_TRUE(output.find("Latency") != std::string::npos);
    EXPECT_TRUE(output.find("CPU Usage") != std::string::npos);
    EXPECT_TRUE(output.find("Memory Usage") != std::string::npos);
}

// Test with empty file
TEST_F(ResultVisualizerTest, EmptyFile) {
    // Create an empty input file with just the header
    std::filesystem::path empty_file_path = test_dir_ / "empty.csv";
    std::ofstream file(empty_file_path);
    file << "Timestamp,Label,Value,Units\n";
    file.close();
    
    // Create a visualizer with the empty file
    ResultVisualizer visualizer(empty_file_path.string());
    
    // Capture stdout during visualization
    std::string output = CaptureStdout([&visualizer]() {
        visualizer.Visualize();
    });
    
    // Check that the output contains a message about no data points
    EXPECT_TRUE(output.find("No data points found") != std::string::npos);
}

// Test with malformed file
TEST_F(ResultVisualizerTest, MalformedFile) {
    // Create a malformed input file
    std::filesystem::path malformed_file_path = test_dir_ / "malformed.csv";
    std::ofstream file(malformed_file_path);
    file << "This,Is,Not,Valid\n";
    file << "Not,A,Proper,Header\n";
    file.close();
    
    // Create a visualizer with the malformed file
    ResultVisualizer visualizer(malformed_file_path.string());
    
    // Capture stdout during visualization
    std::string output = CaptureStdout([&visualizer]() {
        visualizer.Visualize();
    });
    
    // The visualizer should handle the malformed file gracefully
    // It might either show an error or show the raw data as is
    // We just check that it doesn't crash and produces some output
    EXPECT_FALSE(output.empty());
}

// Test with real data file created by DataCollector
TEST_F(ResultVisualizerTest, RealDataFile) {
    // Create a data file using DataCollector
    std::filesystem::path data_file_path = test_dir_ / "real_data.csv";
    {
        DataCollector collector(data_file_path.string());
        
        // Collect some test data
        collector.CollectDataPoint("Throughput", 1234.56, "MB/s");
        collector.CollectDataPoint("IOPS", 300000, "ops/s");
        collector.CollectDataPoint("Latency", 85.2, "µs");
        
        // The collector will be automatically destroyed when this scope ends
    }
    
    // Create a visualizer with the data file
    ResultVisualizer visualizer(data_file_path.string());
    
    // Capture stdout during visualization
    std::string output = CaptureStdout([&visualizer]() {
        visualizer.Visualize();
    });
    
    // Check that the output contains the data we collected
    EXPECT_TRUE(output.find("Throughput") != std::string::npos);
    EXPECT_TRUE(output.find("IOPS") != std::string::npos);
    EXPECT_TRUE(output.find("Latency") != std::string::npos);
}