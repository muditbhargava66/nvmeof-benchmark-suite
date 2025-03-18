#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../../include/utils/nvmeof_utils.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <thread>

using namespace nvmeof::utils;

class NvmeofUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test directory
        test_dir_ = std::filesystem::temp_directory_path() / "nvmeof_test";
        std::filesystem::create_directories(test_dir_);
        
        // Create test file
        test_file_path_ = test_dir_ / "test_file.txt";
        CreateTestFile(test_file_path_, test_content_);
    }
    
    void TearDown() override {
        // Clean up test files and directory
        std::filesystem::remove_all(test_dir_);
    }
    
    // Helper to create a test file
    void CreateTestFile(const std::filesystem::path& path, const std::string& content) {
        std::ofstream file(path);
        ASSERT_TRUE(file.is_open());
        file << content;
        file.close();
    }
    
    std::filesystem::path test_dir_;
    std::filesystem::path test_file_path_;
    const std::string test_content_ = "This is a test file.\nIt has multiple lines.\nEnd of file.";
};

// Test SplitString method
TEST_F(NvmeofUtilsTest, SplitString) {
    // Test with single delimiter
    std::string str = "a,b,c,d";
    auto tokens = SplitString(str, ',');
    
    ASSERT_EQ(4, tokens.size());
    EXPECT_EQ("a", tokens[0]);
    EXPECT_EQ("b", tokens[1]);
    EXPECT_EQ("c", tokens[2]);
    EXPECT_EQ("d", tokens[3]);
    
    // Test with multiple consecutive delimiters
    str = "a,,b,c";
    tokens = SplitString(str, ',');
    
    ASSERT_EQ(4, tokens.size());
    EXPECT_EQ("a", tokens[0]);
    EXPECT_EQ("", tokens[1]);
    EXPECT_EQ("b", tokens[2]);
    EXPECT_EQ("c", tokens[3]);
    
    // Test with empty string
    str = "";
    tokens = SplitString(str, ',');
    
    ASSERT_EQ(1, tokens.size());
    EXPECT_EQ("", tokens[0]);
    
    // Test with string containing only delimiters
    str = ",,,";
    tokens = SplitString(str, ',');
    
    ASSERT_EQ(4, tokens.size());
    EXPECT_EQ("", tokens[0]);
    EXPECT_EQ("", tokens[1]);
    EXPECT_EQ("", tokens[2]);
    EXPECT_EQ("", tokens[3]);
}

// Test TrimString method
TEST_F(NvmeofUtilsTest, TrimString) {
    // Test with whitespace on both ends
    std::string str = "  \t Hello, World! \n ";
    EXPECT_EQ("Hello, World!", TrimString(str));
    
    // Test with no whitespace
    str = "Hello, World!";
    EXPECT_EQ("Hello, World!", TrimString(str));
    
    // Test with only whitespace
    str = "   \t\n  ";
    EXPECT_EQ("", TrimString(str));
    
    // Test with empty string
    str = "";
    EXPECT_EQ("", TrimString(str));
}

// Test FileExists method
TEST_F(NvmeofUtilsTest, FileExists) {
    // Test with existing file
    EXPECT_TRUE(FileExists(test_file_path_));
    
    // Test with non-existent file
    std::filesystem::path non_existent_file = test_dir_ / "non_existent.txt";
    EXPECT_FALSE(FileExists(non_existent_file));
    
    // Test with directory
    EXPECT_TRUE(DirectoryExists(test_dir_));
    
    // Test with non-existent directory
    std::filesystem::path non_existent_dir = test_dir_ / "non_existent_dir";
    EXPECT_FALSE(DirectoryExists(non_existent_dir));
}

// Test CreateDirectory method
TEST_F(NvmeofUtilsTest, CreateDirectory) {
    // Test creating a new directory
    std::filesystem::path new_dir = test_dir_ / "new_dir";
    EXPECT_TRUE(CreateDirectory(new_dir));
    EXPECT_TRUE(DirectoryExists(new_dir));
    
    // Test creating an existing directory (should return true)
    EXPECT_TRUE(CreateDirectory(new_dir));
    
    // Test creating nested directories
    std::filesystem::path nested_dir = test_dir_ / "nested" / "dirs" / "here";
    EXPECT_TRUE(CreateDirectory(nested_dir));
    EXPECT_TRUE(DirectoryExists(nested_dir));
}

// Test RemoveFile method
TEST_F(NvmeofUtilsTest, RemoveFile) {
    // Test removing an existing file
    EXPECT_TRUE(RemoveFile(test_file_path_));
    EXPECT_FALSE(FileExists(test_file_path_));
    
    // Test removing a non-existent file
    std::filesystem::path non_existent_file = test_dir_ / "non_existent.txt";
    EXPECT_FALSE(RemoveFile(non_existent_file));
}

// Test ReadFileToString method
TEST_F(NvmeofUtilsTest, ReadFileToString) {
    // Test reading an existing file
    std::string content = ReadFileToString(test_file_path_);
    EXPECT_EQ(test_content_, content);
    
    // Test reading a non-existent file
    std::filesystem::path non_existent_file = test_dir_ / "non_existent.txt";
    content = ReadFileToString(non_existent_file);
    EXPECT_TRUE(content.empty());
}

// Test WriteStringToFile method
TEST_F(NvmeofUtilsTest, WriteStringToFile) {
    // Test writing to a new file
    std::filesystem::path new_file = test_dir_ / "new_file.txt";
    std::string new_content = "This is a new file.";
    EXPECT_TRUE(WriteStringToFile(new_file, new_content));
    
    // Verify the content was written
    std::string read_content = ReadFileToString(new_file);
    EXPECT_EQ(new_content, read_content);
    
    // Test overwriting an existing file
    new_content = "This content overwrites the previous content.";
    EXPECT_TRUE(WriteStringToFile(new_file, new_content));
    
    // Verify the content was overwritten
    read_content = ReadFileToString(new_file);
    EXPECT_EQ(new_content, read_content);
}

// Test AppendStringToFile method
TEST_F(NvmeofUtilsTest, AppendStringToFile) {
    // Test appending to an existing file
    std::string append_content = "\nThis content is appended.";
    EXPECT_TRUE(AppendStringToFile(test_file_path_, append_content));
    
    // Verify the content was appended
    std::string read_content = ReadFileToString(test_file_path_);
    EXPECT_EQ(test_content_ + append_content, read_content);
    
    // Test appending to a non-existent file (should create it)
    std::filesystem::path new_file = test_dir_ / "append_new_file.txt";
    std::string new_content = "This is a new file created by append.";
    EXPECT_TRUE(AppendStringToFile(new_file, new_content));
    
    // Verify the content was written
    read_content = ReadFileToString(new_file);
    EXPECT_EQ(new_content, read_content);
}

// Test FormatTimestamp method
TEST_F(NvmeofUtilsTest, FormatTimestamp) {
    // Get current time
    auto now = std::chrono::system_clock::now();
    
    // Format with default format
    std::string timestamp = FormatTimestamp(now);
    
    // Check length (not exact since it can vary, but should be reasonable)
    EXPECT_GT(timestamp.length(), 10);
    
    // Format with custom format
    timestamp = FormatTimestamp(now, "%Y%m%d");
    
    // Should be exactly 8 characters (YYYYMMDD)
    EXPECT_EQ(8, timestamp.length());
    
    // Check that the year part is reasonable (between 2020 and 2100)
    int year = std::stoi(timestamp.substr(0, 4));
    EXPECT_GE(year, 2020);
    EXPECT_LE(year, 2100);
}

// Test GetCurrentTimestamp method
TEST_F(NvmeofUtilsTest, GetCurrentTimestamp) {
    // Get current timestamp with default format
    std::string timestamp = GetCurrentTimestamp();
    
    // Check length (not exact since it can vary, but should be reasonable)
    EXPECT_GT(timestamp.length(), 10);
    
    // Get current timestamp with custom format
    timestamp = GetCurrentTimestamp("%Y%m%d");
    
    // Should be exactly 8 characters (YYYYMMDD)
    EXPECT_EQ(8, timestamp.length());
    
    // Check that the year part is reasonable (between 2020 and 2100)
    int year = std::stoi(timestamp.substr(0, 4));
    EXPECT_GE(year, 2020);
    EXPECT_LE(year, 2100);
}

// Test FormatByteSize method
TEST_F(NvmeofUtilsTest, FormatByteSize) {
    // Test various byte sizes
    EXPECT_EQ("0.00 B", FormatByteSize(0));
    EXPECT_EQ("1.00 B", FormatByteSize(1));
    EXPECT_EQ("1.00 KB", FormatByteSize(1024));
    EXPECT_EQ("1.50 KB", FormatByteSize(1536));
    EXPECT_EQ("1.00 MB", FormatByteSize(1024 * 1024));
    EXPECT_EQ("1.00 GB", FormatByteSize(1024 * 1024 * 1024));
    EXPECT_EQ("1.00 TB", FormatByteSize(1024ULL * 1024ULL * 1024ULL * 1024ULL));
    
    // Test with very large value
    EXPECT_EQ("1.00 PB", FormatByteSize(1024ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL));
}

// Test GenerateRandomNumber method
TEST_F(NvmeofUtilsTest, GenerateRandomNumber) {
    // Test with specific range
    const uint32_t min = 10;
    const uint32_t max = 20;
    
    // Generate 100 random numbers and check they're all within range
    for (int i = 0; i < 100; ++i) {
        uint32_t random = GenerateRandomNumber(min, max);
        EXPECT_GE(random, min);
        EXPECT_LE(random, max);
    }
    
    // Test with single value range
    uint32_t random = GenerateRandomNumber(42, 42);
    EXPECT_EQ(42, random);
}

// Test GenerateUniqueId method
TEST_F(NvmeofUtilsTest, GenerateUniqueId) {
    // Generate multiple IDs and make sure they're unique
    const int num_ids = 100;
    std::vector<std::string> ids;
    
    for (int i = 0; i < num_ids; ++i) {
        ids.push_back(GenerateUniqueId());
    }
    
    // Check uniqueness
    std::sort(ids.begin(), ids.end());
    auto duplicate = std::adjacent_find(ids.begin(), ids.end());
    EXPECT_EQ(ids.end(), duplicate) << "Duplicate ID found: " << *duplicate;
    
    // Check ID format (should be a hex string)
    for (const auto& id : ids) {
        EXPECT_TRUE(std::all_of(id.begin(), id.end(), [](char c) {
            return std::isxdigit(c);
        })) << "ID contains non-hex characters: " << id;
    }
}

// Test ValidateIpAddress method
TEST_F(NvmeofUtilsTest, ValidateIpAddress) {
    // Test valid IP addresses
    EXPECT_TRUE(ValidateIpAddress("192.168.1.1"));
    EXPECT_TRUE(ValidateIpAddress("10.0.0.1"));
    EXPECT_TRUE(ValidateIpAddress("172.16.0.1"));
    EXPECT_TRUE(ValidateIpAddress("255.255.255.255"));
    EXPECT_TRUE(ValidateIpAddress("0.0.0.0"));
    
    // Test invalid IP addresses
    EXPECT_FALSE(ValidateIpAddress(""));
    EXPECT_FALSE(ValidateIpAddress("192.168.1"));
    EXPECT_FALSE(ValidateIpAddress("192.168.1."));
    EXPECT_FALSE(ValidateIpAddress("192.168.1.256"));
    EXPECT_FALSE(ValidateIpAddress("192.168.1.1.1"));
    EXPECT_FALSE(ValidateIpAddress("192.168.1.a"));
    EXPECT_FALSE(ValidateIpAddress("192.168.1.-1"));
    EXPECT_FALSE(ValidateIpAddress("not an ip address"));
}

// Test ReadEnvironmentVariable method
TEST_F(NvmeofUtilsTest, ReadEnvironmentVariable) {
    // Set an environment variable for testing
    SetEnvironmentVariable("NVMEOF_TEST_VAR", "test_value");
    
    // Read the environment variable
    std::string value = ReadEnvironmentVariable("NVMEOF_TEST_VAR");
    EXPECT_EQ("test_value", value);
    
    // Test with default value for non-existent variable
    value = ReadEnvironmentVariable("NVMEOF_NON_EXISTENT_VAR", "default_value");
    EXPECT_EQ("default_value", value);
}

// Test ParseBooleanString method
TEST_F(NvmeofUtilsTest, ParseBooleanString) {
    // Test true values
    EXPECT_TRUE(ParseBooleanString("true"));
    EXPECT_TRUE(ParseBooleanString("True"));
    EXPECT_TRUE(ParseBooleanString("TRUE"));
    EXPECT_TRUE(ParseBooleanString("yes"));
    EXPECT_TRUE(ParseBooleanString("Yes"));
    EXPECT_TRUE(ParseBooleanString("YES"));
    EXPECT_TRUE(ParseBooleanString("1"));
    EXPECT_TRUE(ParseBooleanString("on"));
    EXPECT_TRUE(ParseBooleanString("On"));
    EXPECT_TRUE(ParseBooleanString("ON"));
    
    // Test false values
    EXPECT_FALSE(ParseBooleanString("false"));
    EXPECT_FALSE(ParseBooleanString("False"));
    EXPECT_FALSE(ParseBooleanString("FALSE"));
    EXPECT_FALSE(ParseBooleanString("no"));
    EXPECT_FALSE(ParseBooleanString("No"));
    EXPECT_FALSE(ParseBooleanString("NO"));
    EXPECT_FALSE(ParseBooleanString("0"));
    EXPECT_FALSE(ParseBooleanString("off"));
    EXPECT_FALSE(ParseBooleanString("Off"));
    EXPECT_FALSE(ParseBooleanString("OFF"));
    
    // Test invalid values (should default to false)
    EXPECT_FALSE(ParseBooleanString(""));
    EXPECT_FALSE(ParseBooleanString("maybe"));
    EXPECT_FALSE(ParseBooleanString("not a boolean"));
}

// Test thread safety of file operations
TEST_F(NvmeofUtilsTest, ThreadSafety) {
    // Create a file for testing
    std::filesystem::path thread_test_file = test_dir_ / "thread_test.txt";
    WriteStringToFile(thread_test_file, "");
    
    // Create multiple threads to append to the file
    const int num_threads = 10;
    const int appends_per_thread = 100;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([i, &thread_test_file]() {
            for (int j = 0; j < appends_per_thread; ++j) {
                std::string content = "Thread " + std::to_string(i) + 
                                      " Append " + std::to_string(j) + "\n";
                AppendStringToFile(thread_test_file, content);
                
                // Small sleep to increase chance of thread interleaving
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Read the file and count the number of lines
    std::string content = ReadFileToString(thread_test_file);
    int line_count = std::count(content.begin(), content.end(), '\n');
    
    // There should be exactly num_threads * appends_per_thread lines
    EXPECT_EQ(num_threads * appends_per_thread, line_count);
}