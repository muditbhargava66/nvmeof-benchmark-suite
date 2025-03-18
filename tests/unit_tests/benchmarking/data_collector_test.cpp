#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../../include/benchmarking/data_collector.h"
#include <fstream>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace nvmeof::benchmarking;

class DataCollectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test directory
        test_dir_ = std::filesystem::temp_directory_path() / "nvmeof_test";
        std::filesystem::create_directories(test_dir_);
        
        // Set up test file paths
        csv_file_path_ = test_dir_ / "test_data.csv";
        json_file_path_ = test_dir_ / "test_data.json";
        text_file_path_ = test_dir_ / "test_data.txt";
    }
    
    void TearDown() override {
        // Clean up test files and directory
        std::filesystem::remove_all(test_dir_);
    }
    
    std::filesystem::path test_dir_;
    std::filesystem::path csv_file_path_;
    std::filesystem::path json_file_path_;
    std::filesystem::path text_file_path_;
    
    // Helper to read a file into a string
    std::string ReadFileContents(const std::filesystem::path& file_path) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return "";
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return content;
    }
};

// Test constructor with invalid file path
TEST_F(DataCollectorTest, ConstructorInvalidPath) {
    // Try to create a collector with an invalid file path
    std::filesystem::path invalid_path = "/invalid/path/that/does/not/exist/test.csv";
    
    EXPECT_THROW(
        DataCollector collector(invalid_path.string()),
        std::runtime_error
    );
}

// Test CSV format output
TEST_F(DataCollectorTest, CsvFormatOutput) {
    // Create a collector with CSV format
    DataCollector collector(csv_file_path_.string(), OutputFormat::CSV);
    
    // Collect some test data
    EXPECT_TRUE(collector.CollectDataPoint("Test Point 1", 123.45, "MB/s"));
    EXPECT_TRUE(collector.CollectDataPoint("Test Point 2", 67.89, "ms"));
    
    // Check the data point count
    EXPECT_EQ(2, collector.GetDataPointCount());
    
    // Flush the data
    EXPECT_TRUE(collector.Flush());
    
    // Read the file contents
    std::string content = ReadFileContents(csv_file_path_);
    
    // Check that the content is as expected
    EXPECT_TRUE(content.find("Timestamp,Label,Value,Units") != std::string::npos);
    EXPECT_TRUE(content.find("Test Point 1") != std::string::npos);
    EXPECT_TRUE(content.find("123.45") != std::string::npos);
    EXPECT_TRUE(content.find("MB/s") != std::string::npos);
    EXPECT_TRUE(content.find("Test Point 2") != std::string::npos);
    EXPECT_TRUE(content.find("67.89") != std::string::npos);
    EXPECT_TRUE(content.find("ms") != std::string::npos);
}

// Test JSON format output
TEST_F(DataCollectorTest, JsonFormatOutput) {
    // Create a collector with JSON format
    {
        DataCollector collector(json_file_path_.string(), OutputFormat::JSON);
        
        // Collect some test data
        EXPECT_TRUE(collector.CollectDataPoint("Test Point 1", 123.45, "MB/s"));
        EXPECT_TRUE(collector.CollectDataPoint("Test Point 2", 67.89, "ms"));
        
        // The collector will be automatically destroyed when this scope ends,
        // which will properly close and flush the JSON file
    }
    
    // Read the file contents
    std::string content = ReadFileContents(json_file_path_);
    
    // Check that the content is as expected
    EXPECT_TRUE(content.find("\"data_points\"") != std::string::npos);
    EXPECT_TRUE(content.find("\"label\": \"Test Point 1\"") != std::string::npos);
    EXPECT_TRUE(content.find("\"value\": 123.45") != std::string::npos);
    EXPECT_TRUE(content.find("\"units\": \"MB/s\"") != std::string::npos);
    EXPECT_TRUE(content.find("\"label\": \"Test Point 2\"") != std::string::npos);
    EXPECT_TRUE(content.find("\"value\": 67.89") != std::string::npos);
    EXPECT_TRUE(content.find("\"units\": \"ms\"") != std::string::npos);
}

// Test plain text format output
TEST_F(DataCollectorTest, PlainTextFormatOutput) {
    // Create a collector with plain text format
    {
        DataCollector collector(text_file_path_.string(), OutputFormat::PLAINTEXT);
        
        // Collect some test data
        EXPECT_TRUE(collector.CollectDataPoint("Test Point 1", 123.45, "MB/s"));
        EXPECT_TRUE(collector.CollectDataPoint("Test Point 2", 67.89, "ms"));
        
        // The collector will be automatically destroyed when this scope ends,
        // which will properly close and flush the text file
    }
    
    // Read the file contents
    std::string content = ReadFileContents(text_file_path_);
    
    // Check that the content is as expected
    EXPECT_TRUE(content.find("=== NVMe-oF Benchmark Data ===") != std::string::npos);
    EXPECT_TRUE(content.find("Test Point 1") != std::string::npos);
    EXPECT_TRUE(content.find("123.45") != std::string::npos);
    EXPECT_TRUE(content.find("MB/s") != std::string::npos);
    EXPECT_TRUE(content.find("Test Point 2") != std::string::npos);
    EXPECT_TRUE(content.find("67.89") != std::string::npos);
    EXPECT_TRUE(content.find("ms") != std::string::npos);
    EXPECT_TRUE(content.find("Total data points: 2") != std::string::npos);
}

// Test legacy CollectData method
TEST_F(DataCollectorTest, LegacyCollectData) {
    // Create a collector
    {
        DataCollector collector(csv_file_path_.string(), OutputFormat::CSV);
        
        // Use the legacy method
        // Using deprecated function intentionally to test backward compatibility
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wdeprecated-declarations"
        EXPECT_TRUE(collector.CollectData("Legacy data point"));
        #pragma clang diagnostic pop
        
        // Check the data point count
        EXPECT_EQ(1, collector.GetDataPointCount());
        
        // The collector will be automatically destroyed when this scope ends
    }
    
    // Read the file contents
    std::string content = ReadFileContents(csv_file_path_);
    
    // Check that the content contains the legacy data
    EXPECT_TRUE(content.find("Legacy data point") != std::string::npos);
}

// Test thread safety with multiple threads writing
TEST_F(DataCollectorTest, ThreadSafety) {
    // Create a collector
    DataCollector collector(csv_file_path_.string(), OutputFormat::CSV);
    
    // Launch multiple threads to write data
    const int num_threads = 5;
    const int points_per_thread = 10;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&collector, i]() {
            for (int j = 0; j < points_per_thread; ++j) {
                std::string label = "Thread " + std::to_string(i) + " Point " + std::to_string(j);
                double value = i * 100.0 + j;
                collector.CollectDataPoint(label, value, "units");
                
                // Small sleep to increase chance of thread interleaving
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Check the total number of data points
    EXPECT_EQ(num_threads * points_per_thread, collector.GetDataPointCount());
}