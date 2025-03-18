#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <fstream>

// Custom test environment class to set up global resources for integration tests
class NvmeofIntegrationTestEnvironment : public ::testing::Environment {
public:
    ~NvmeofIntegrationTestEnvironment() override = default;

    // Override this to define how to set up the environment
    void SetUp() override {
        std::cout << "Setting up NVMe-oF integration test environment..." << std::endl;
        
        // Create test directories
        CreateTestDirectories();
        
        // Create test data files
        CreateTestDataFiles();
        
        std::cout << "Integration test environment setup completed." << std::endl;
    }

    // Override this to define how to tear down the environment
    void TearDown() override {
        std::cout << "Tearing down NVMe-oF integration test environment..." << std::endl;
        
        // Clean up test directories and files
        CleanupTestDirectories();
        
        std::cout << "Integration test environment teardown completed." << std::endl;
    }

private:
    // Create test directories
    void CreateTestDirectories() {
        std::filesystem::create_directories("./test_output");
        std::filesystem::create_directories("./test_output/workload_profiles");
        std::filesystem::create_directories("./test_output/benchmark_results");
        std::filesystem::create_directories("./test_output/analysis_reports");
        std::filesystem::create_directories("./test_output/optimization_configs");
    }
    
    // Create test data files
    void CreateTestDataFiles() {
        // Create a simple workload profile for testing
        std::ofstream workload_profile("./test_output/workload_profiles/test_profile.json");
        workload_profile << R"({
            "name": "Integration Test Profile",
            "description": "Profile for integration testing",
            "total_size": 1048576,
            "block_size": 4096,
            "num_blocks": 256,
            "read_percentage": 70,
            "write_percentage": 30,
            "random_percentage": 50
        })";
        workload_profile.close();
        
        // Create a simple optimization config for testing
        std::ofstream optimization_config("./test_output/optimization_configs/test_config.ini");
        optimization_config << "cpu_bottleneck=cpu_governor=performance\n";
        optimization_config << "memory_bottleneck=vm.swappiness=10\n";
        optimization_config << "network_bottleneck=net.core.rmem_max=16777216\n";
        optimization_config.close();
    }
    
    // Clean up test directories and files
    void CleanupTestDirectories() {
        std::filesystem::remove_all("./test_output");
    }
};

int main(int argc, char** argv) {
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // Create and register the custom environment
    ::testing::AddGlobalTestEnvironment(new NvmeofIntegrationTestEnvironment);
    
    // Print test banner
    std::cout << "=======================================" << std::endl;
    std::cout << "Running NVMe-oF Integration Tests" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    // Run the tests
    int result = RUN_ALL_TESTS();
    
    // Print test summary
    std::cout << "=======================================" << std::endl;
    std::cout << "Integration tests completed with exit code: " << result << std::endl;
    std::cout << "=======================================" << std::endl;
    
    return result;
}