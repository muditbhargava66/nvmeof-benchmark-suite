#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../../include/optimization_engine/config_knowledge_base.h"
#include <fstream>
#include <filesystem>
#include <string>

using namespace nvmeof::optimization_engine;

class ConfigKnowledgeBaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test directory
        test_dir_ = std::filesystem::temp_directory_path() / "nvmeof_test";
        std::filesystem::create_directories(test_dir_);
        
        // Create a test config file
        config_file_path_ = test_dir_ / "test_config.ini";
        CreateTestConfigFile();
    }
    
    void TearDown() override {
        // Clean up test files and directory
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
    
    std::filesystem::path test_dir_;
    std::filesystem::path config_file_path_;
};

// Test constructor with valid config file
TEST_F(ConfigKnowledgeBaseTest, Constructor) {
    // Create a knowledge base with a valid config file
    ConfigKnowledgeBase kb(config_file_path_.string());
    
    // Check that it loaded the configuration
    EXPECT_FALSE(kb.GetConfigValue("cpu_bottleneck").empty());
    EXPECT_FALSE(kb.GetConfigValue("memory_bottleneck").empty());
    EXPECT_FALSE(kb.GetConfigValue("network_bottleneck").empty());
    EXPECT_FALSE(kb.GetConfigValue("storage_bottleneck").empty());
}

// Test constructor with non-existent config file
TEST_F(ConfigKnowledgeBaseTest, ConstructorNonExistentFile) {
    // Non-existent file should log an error but not throw
    std::filesystem::path non_existent_file = test_dir_ / "non_existent.ini";
    
    // Capture stderr to check for error message
    testing::internal::CaptureStderr();
    
    // Create a knowledge base with a non-existent file
    ConfigKnowledgeBase kb(non_existent_file.string());
    
    // Get stderr output
    std::string stderr_output = testing::internal::GetCapturedStderr();
    
    // Check for error message
    EXPECT_TRUE(stderr_output.find("Failed to open config file") != std::string::npos);
    
    // Get config value should return empty string
    EXPECT_TRUE(kb.GetConfigValue("cpu_bottleneck").empty());
}

// Test GetConfigValue method
TEST_F(ConfigKnowledgeBaseTest, GetConfigValue) {
    ConfigKnowledgeBase kb(config_file_path_.string());
    
    // Get values for existing keys
    EXPECT_EQ("cpu_governor=performance,hugepages=1024", kb.GetConfigValue("cpu_bottleneck"));
    EXPECT_EQ("vm.swappiness=10,vm.vfs_cache_pressure=50", kb.GetConfigValue("memory_bottleneck"));
    EXPECT_EQ("net.core.rmem_max=16777216,net.core.wmem_max=16777216", kb.GetConfigValue("network_bottleneck"));
    EXPECT_EQ("vm.dirty_ratio=10,vm.dirty_background_ratio=5", kb.GetConfigValue("storage_bottleneck"));
    
    // Get value for non-existent key
    EXPECT_TRUE(kb.GetConfigValue("non_existent_key").empty());
}

// Test with empty config file
TEST_F(ConfigKnowledgeBaseTest, EmptyConfigFile) {
    // Create an empty config file
    std::filesystem::path empty_file_path = test_dir_ / "empty.ini";
    std::ofstream file(empty_file_path);
    file.close();
    
    // Create a knowledge base with the empty file
    ConfigKnowledgeBase kb(empty_file_path.string());
    
    // All config values should be empty
    EXPECT_TRUE(kb.GetConfigValue("cpu_bottleneck").empty());
    EXPECT_TRUE(kb.GetConfigValue("memory_bottleneck").empty());
    EXPECT_TRUE(kb.GetConfigValue("network_bottleneck").empty());
    EXPECT_TRUE(kb.GetConfigValue("storage_bottleneck").empty());
}

// Test with malformed config file
TEST_F(ConfigKnowledgeBaseTest, MalformedConfigFile) {
    // Create a malformed config file
    std::filesystem::path malformed_file_path = test_dir_ / "malformed.ini";
    std::ofstream file(malformed_file_path);
    file << "This is not a valid config file\n";
    file << "No key-value pairs here\n";
    file << "cpu_bottleneck\n";  // Missing value
    file << "=value\n";          // Missing key
    file.close();
    
    // Create a knowledge base with the malformed file
    ConfigKnowledgeBase kb(malformed_file_path.string());
    
    // All valid config values should be loaded, invalid lines should be skipped
    EXPECT_TRUE(kb.GetConfigValue("cpu_bottleneck").empty());  // Malformed key
    EXPECT_TRUE(kb.GetConfigValue("").empty());               // Empty key
    EXPECT_TRUE(kb.GetConfigValue("This").empty());           // Not a key-value pair
}

// Test with duplicate keys
TEST_F(ConfigKnowledgeBaseTest, DuplicateKeys) {
    // Create a config file with duplicate keys
    std::filesystem::path duplicate_file_path = test_dir_ / "duplicate.ini";
    std::ofstream file(duplicate_file_path);
    file << "cpu_bottleneck=cpu_governor=performance\n";
    file << "cpu_bottleneck=hugepages=1024\n";  // Duplicate key
    file.close();
    
    // Create a knowledge base with the duplicate file
    ConfigKnowledgeBase kb(duplicate_file_path.string());
    
    // The last value for a key should be used
    EXPECT_EQ("hugepages=1024", kb.GetConfigValue("cpu_bottleneck"));
}

// Test with comments and whitespace
TEST_F(ConfigKnowledgeBaseTest, CommentsAndWhitespace) {
    // Create a config file with comments and whitespace
    std::filesystem::path commented_file_path = test_dir_ / "commented.ini";
    std::ofstream file(commented_file_path);
    file << "# This is a comment\n";
    file << "cpu_bottleneck = cpu_governor=performance  # Trailing comment\n";
    file << "  memory_bottleneck  =  vm.swappiness=10  \n";  // Leading/trailing whitespace
    file << "\n";  // Empty line
    file << "# Another comment\n";
    file.close();
    
    // Create a knowledge base with the commented file
    ConfigKnowledgeBase kb(commented_file_path.string());
    
    // Comments should be ignored, whitespace should be trimmed
    EXPECT_EQ("cpu_governor=performance", kb.GetConfigValue("cpu_bottleneck"));
    EXPECT_EQ("vm.swappiness=10", kb.GetConfigValue("memory_bottleneck"));
}

// Test LoadConfigFile method (indirectly through constructor)
TEST_F(ConfigKnowledgeBaseTest, LoadConfigFile) {
    // Create a config file for loading
    std::filesystem::path load_file_path = test_dir_ / "load.ini";
    std::ofstream file(load_file_path);
    file << "test_key=test_value\n";
    file.close();
    
    // Create a knowledge base with the file
    ConfigKnowledgeBase kb(load_file_path.string());
    
    // Check that the file was loaded
    EXPECT_EQ("test_value", kb.GetConfigValue("test_key"));
}