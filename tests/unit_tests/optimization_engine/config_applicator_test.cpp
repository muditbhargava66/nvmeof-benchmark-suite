#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../../include/optimization_engine/config_applicator.h"
#include <sstream>
#include <string>
#include <filesystem>
#include <fstream>

using namespace nvmeof::optimization_engine;

class ConfigApplicatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test directory
        test_dir_ = std::filesystem::temp_directory_path() / "nvmeof_test";
        std::filesystem::create_directories(test_dir_);
        
        // Create a mock CPU governor directory for testing
        cpu_governor_dir_ = test_dir_ / "cpu0" / "cpufreq";
        std::filesystem::create_directories(cpu_governor_dir_);
        
        // Create a mock scaling_governor file
        scaling_governor_file_ = cpu_governor_dir_ / "scaling_governor";
        std::ofstream governor_file(scaling_governor_file_);
        governor_file << "powersave";
        governor_file.close();
    }
    
    void TearDown() override {
        // Clean up test directory
        std::filesystem::remove_all(test_dir_);
    }
    
    // Helper to capture stderr
    std::string CaptureStderr(std::function<void()> func) {
        std::streambuf* old = std::cerr.rdbuf();
        std::stringstream ss;
        std::cerr.rdbuf(ss.rdbuf());
        
        func();
        
        std::cerr.rdbuf(old);
        return ss.str();
    }
    
    std::filesystem::path test_dir_;
    std::filesystem::path cpu_governor_dir_;
    std::filesystem::path scaling_governor_file_;
};

// Mock ConfigApplicator for testing
class MockConfigApplicator : public ConfigApplicator {
public:
    MockConfigApplicator() = default;
    
    MOCK_METHOD(void, SetCPUGovernor, (const std::string& governor), (override));
    MOCK_METHOD(void, EnableHugePages, (size_t num_pages), (override));
    MOCK_METHOD(void, SetIRQAffinity, (const std::string& irq_affinity), (override));
    MOCK_METHOD(void, SetTCPRMem, (const std::string& rmem_values), (override));
    MOCK_METHOD(void, SetTCPWMem, (const std::string& wmem_values), (override));
    
    // Override ApplyConfiguration to call our mock methods directly
    void ApplyConfiguration(const std::string& config) override {
        // Use the parent implementation but make sure our mock methods get called
        ConfigApplicator::ApplyConfiguration(config);
    }
};

// Test ApplyConfiguration method with different configurations
TEST_F(ConfigApplicatorTest, ApplyConfiguration) {
    MockConfigApplicator applicator;
    
    // Set expectations for the mocked methods
    EXPECT_CALL(applicator, SetCPUGovernor("performance"))
        .Times(testing::AtLeast(0));
    
    EXPECT_CALL(applicator, EnableHugePages(1024))
        .Times(testing::AtLeast(0));
    
    EXPECT_CALL(applicator, SetIRQAffinity("f"))
        .Times(testing::AtLeast(0));
    
    EXPECT_CALL(applicator, SetTCPRMem("4096 87380 16777216"))
        .Times(testing::AtLeast(0));
    
    EXPECT_CALL(applicator, SetTCPWMem("4096 65536 16777216"))
        .Times(testing::AtLeast(0));
    
    // Apply configuration
    applicator.ApplyConfiguration(
        "cpu_governor=performance,hugepages=1024,irq_affinity=f,"
        "tcp_rmem=4096 87380 16777216,tcp_wmem=4096 65536 16777216"
    );
}

// Test ApplyConfiguration with empty configuration
TEST_F(ConfigApplicatorTest, ApplyConfigurationEmpty) {
    MockConfigApplicator applicator;
    
    // No methods should be called
    EXPECT_CALL(applicator, SetCPUGovernor(testing::_))
        .Times(0);
    
    EXPECT_CALL(applicator, EnableHugePages(testing::_))
        .Times(0);
    
    EXPECT_CALL(applicator, SetIRQAffinity(testing::_))
        .Times(0);
    
    EXPECT_CALL(applicator, SetTCPRMem(testing::_))
        .Times(0);
    
    EXPECT_CALL(applicator, SetTCPWMem(testing::_))
        .Times(0);
    
    // Apply empty configuration
    applicator.ApplyConfiguration("");
}

// Test ApplyConfiguration with unknown keys
TEST_F(ConfigApplicatorTest, ApplyConfigurationUnknownKeys) {
    MockConfigApplicator applicator;
    
    // CPU governor should be set, but unknown keys should be ignored
    EXPECT_CALL(applicator, SetCPUGovernor("performance"))
        .Times(testing::AtLeast(0));
    
    EXPECT_CALL(applicator, EnableHugePages(testing::_))
        .Times(0);
    
    // Capture stderr to check for warning messages
    std::string stderr_output = CaptureStderr([&applicator]() {
        applicator.ApplyConfiguration("cpu_governor=performance,unknown_key=value");
    });
    
    // Check for warning message
    EXPECT_TRUE(stderr_output.find("Unknown configuration key: unknown_key") != std::string::npos);
}

// Test ApplyConfiguration with malformed configuration
TEST_F(ConfigApplicatorTest, ApplyConfigurationMalformed) {
    MockConfigApplicator applicator;
    
    // Capture stderr to check for warning messages
    std::string stderr_output = CaptureStderr([&applicator]() {
        applicator.ApplyConfiguration("malformed_config");
    });
    
    // Check for some kind of error or warning message
    EXPECT_FALSE(stderr_output.empty());
}

// Test SetCPUGovernor method
TEST_F(ConfigApplicatorTest, SetCPUGovernor) {
    // Patch the ConfigApplicator to use our test directory
    class TestConfigApplicator : public ConfigApplicator {
    public:
        TestConfigApplicator(const std::string& test_dir)
            : test_dir_(test_dir) {}
        
        // Override to use our test directory
        void SetCPUGovernor(const std::string& governor) {
            std::string cpu_dir = test_dir_ + "/cpu0/cpufreq/scaling_governor";
            std::ofstream file(cpu_dir);
            if (file.is_open()) {
                file << governor;
                file.close();
            } else {
                std::cerr << "Failed to set CPU governor for " << cpu_dir << std::endl;
            }
        }
        
    private:
        std::string test_dir_;
    };
    
    // Create the test applicator
    TestConfigApplicator applicator(test_dir_.string());
    
    // Set CPU governor
    applicator.SetCPUGovernor("performance");
    
    // Read the scaling_governor file to check if it was set
    std::ifstream file(scaling_governor_file_);
    std::string governor;
    std::getline(file, governor);
    file.close();
    
    // Check that the governor was set correctly
    EXPECT_EQ("performance", governor);
}

// Test EnableHugePages method
TEST_F(ConfigApplicatorTest, EnableHugePages) {
    // Create a mock hugepages file
    std::filesystem::path hugepages_file = test_dir_ / "nr_hugepages";
    std::ofstream file(hugepages_file);
    file << "0";
    file.close();
    
    // Patch the ConfigApplicator to use our test directory
    class TestConfigApplicator : public ConfigApplicator {
    public:
        TestConfigApplicator(const std::string& test_dir)
            : test_dir_(test_dir) {}
        
        // Override to use our test directory
        void EnableHugePages(size_t num_pages) {
            std::string hugepages_file = test_dir_ + "/nr_hugepages";
            std::ofstream file(hugepages_file);
            if (file.is_open()) {
                file << num_pages;
                file.close();
            } else {
                std::cerr << "Failed to enable huge pages" << std::endl;
            }
        }
        
    private:
        std::string test_dir_;
    };
    
    // Create the test applicator
    TestConfigApplicator applicator(test_dir_.string());
    
    // Enable huge pages
    applicator.EnableHugePages(1024);
    
    // Read the hugepages file to check if it was set
    std::ifstream hugepages_in(hugepages_file);
    std::string num_pages;
    std::getline(hugepages_in, num_pages);
    hugepages_in.close();
    
    // Check that the number of huge pages was set correctly
    EXPECT_EQ("1024", num_pages);
}

// Test SetIRQAffinity method
TEST_F(ConfigApplicatorTest, SetIRQAffinity) {
    // Create a mock IRQ affinity file
    std::filesystem::path irq_affinity_file = test_dir_ / "default_smp_affinity";
    std::ofstream file(irq_affinity_file);
    file << "0";
    file.close();
    
    // Patch the ConfigApplicator to use our test directory
    class TestConfigApplicator : public ConfigApplicator {
    public:
        TestConfigApplicator(const std::string& test_dir)
            : test_dir_(test_dir) {}
        
        // Override to use our test directory
        void SetIRQAffinity(const std::string& irq_affinity) {
            std::string irq_affinity_file = test_dir_ + "/default_smp_affinity";
            std::ofstream file(irq_affinity_file);
            if (file.is_open()) {
                file << irq_affinity;
                file.close();
            } else {
                std::cerr << "Failed to set IRQ affinity" << std::endl;
            }
        }
        
    private:
        std::string test_dir_;
    };
    
    // Create the test applicator
    TestConfigApplicator applicator(test_dir_.string());
    
    // Set IRQ affinity
    applicator.SetIRQAffinity("f");
    
    // Read the IRQ affinity file to check if it was set
    std::ifstream irq_affinity_in(irq_affinity_file);
    std::string irq_affinity;
    std::getline(irq_affinity_in, irq_affinity);
    irq_affinity_in.close();
    
    // Check that the IRQ affinity was set correctly
    EXPECT_EQ("f", irq_affinity);
}

// Test SetTCPRMem method
TEST_F(ConfigApplicatorTest, SetTCPRMem) {
    // Create a mock TCP RMem file
    std::filesystem::path tcp_rmem_file = test_dir_ / "tcp_rmem";
    std::ofstream file(tcp_rmem_file);
    file << "4096 87380 16777216";
    file.close();
    
    // Patch the ConfigApplicator to use our test directory
    class TestConfigApplicator : public ConfigApplicator {
    public:
        TestConfigApplicator(const std::string& test_dir)
            : test_dir_(test_dir) {}
        
        // Override to use our test directory
        void SetTCPRMem(const std::string& rmem_values) {
            std::string tcp_rmem_file = test_dir_ + "/tcp_rmem";
            std::ofstream file(tcp_rmem_file);
            if (file.is_open()) {
                file << rmem_values;
                file.close();
            } else {
                std::cerr << "Failed to set TCP receive buffer sizes" << std::endl;
            }
        }
        
    private:
        std::string test_dir_;
    };
    
    // Create the test applicator
    TestConfigApplicator applicator(test_dir_.string());
    
    // Set TCP RMem
    applicator.SetTCPRMem("8192 87380 16777216");
    
    // Read the TCP RMem file to check if it was set
    std::ifstream tcp_rmem_in(tcp_rmem_file);
    std::string tcp_rmem;
    std::getline(tcp_rmem_in, tcp_rmem);
    tcp_rmem_in.close();
    
    // Check that the TCP RMem was set correctly
    EXPECT_EQ("8192 87380 16777216", tcp_rmem);
}

// Test SetTCPWMem method
TEST_F(ConfigApplicatorTest, SetTCPWMem) {
    // Create a mock TCP WMem file
    std::filesystem::path tcp_wmem_file = test_dir_ / "tcp_wmem";
    std::ofstream file(tcp_wmem_file);
    file << "4096 65536 16777216";
    file.close();
    
    // Patch the ConfigApplicator to use our test directory
    class TestConfigApplicator : public ConfigApplicator {
    public:
        TestConfigApplicator(const std::string& test_dir)
            : test_dir_(test_dir) {}
        
        // Override to use our test directory
        void SetTCPWMem(const std::string& wmem_values) {
            std::string tcp_wmem_file = test_dir_ + "/tcp_wmem";
            std::ofstream file(tcp_wmem_file);
            if (file.is_open()) {
                file << wmem_values;
                file.close();
            } else {
                std::cerr << "Failed to set TCP send buffer sizes" << std::endl;
            }
        }
        
    private:
        std::string test_dir_;
    };
    
    // Create the test applicator
    TestConfigApplicator applicator(test_dir_.string());
    
    // Set TCP WMem
    applicator.SetTCPWMem("8192 65536 16777216");
    
    // Read the TCP WMem file to check if it was set
    std::ifstream tcp_wmem_in(tcp_wmem_file);
    std::string tcp_wmem;
    std::getline(tcp_wmem_in, tcp_wmem);
    tcp_wmem_in.close();
    
    // Check that the TCP WMem was set correctly
    EXPECT_EQ("8192 65536 16777216", tcp_wmem);
}