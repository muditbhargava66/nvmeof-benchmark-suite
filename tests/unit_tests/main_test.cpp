#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <cstdlib>

// Custom test environment class to set up global resources
class NvmeofTestEnvironment : public ::testing::Environment {
public:
    ~NvmeofTestEnvironment() override = default;

    // Override this to define how to set up the environment
    void SetUp() override {
        std::cout << "Setting up NVMe-oF test environment..." << std::endl;
        
        // Set up any global resources needed for tests here
        // For example, creating test directories, setting environment variables, etc.
        
        // Create temporary directory for test data
        system("mkdir -p ./test_output");
    }

    // Override this to define how to tear down the environment
    void TearDown() override {
        std::cout << "Tearing down NVMe-oF test environment..." << std::endl;
        
        // Clean up any global resources
        
        // Remove temporary test directory
        system("rm -rf ./test_output");
    }
};

int main(int argc, char** argv) {
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // Add custom test output (commented out for now as listeners are not being used)
    // ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    
    // Optionally remove the default XML output listener
    // delete listeners.Release(listeners.default_xml_generator());
    
    // Create and register the custom environment
    ::testing::AddGlobalTestEnvironment(new NvmeofTestEnvironment);
    
    // Print test banner
    std::cout << "=======================================" << std::endl;
    std::cout << "Running NVMe-oF Benchmarking Suite Unit Tests" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    // Run the tests
    int result = RUN_ALL_TESTS();
    
    // Print test summary
    std::cout << "=======================================" << std::endl;
    std::cout << "Unit tests completed with exit code: " << result << std::endl;
    std::cout << "=======================================" << std::endl;
    
    return result;
}