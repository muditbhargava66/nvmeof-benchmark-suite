#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../../include/benchmarking/workload_generator.h"

// Mock for NVMe controller
class MockNvmeCtrlr {
public:
    virtual ~MockNvmeCtrlr() = default;
    virtual int GetNamespaceCount() const = 0;
};

// Mock for NVMe namespace
class MockNvmeNamespace {
public:
    virtual ~MockNvmeNamespace() = default;
    virtual uint32_t GetSectorSize() const = 0;
};

// Test fixture for WorkloadGenerator tests
class WorkloadGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up default workload profile
        profile_.total_size = 1048576;        // 1 MB
        profile_.block_size = 4096;           // 4 KB
        profile_.num_blocks = 256;            // 256 blocks
        profile_.interval_us = 100;           // 100 microseconds
        profile_.read_percentage = 50;        // 50% reads
        profile_.write_percentage = 50;       // 50% writes
        profile_.random_percentage = 70;      // 70% random
    }
    
    nvmeof::benchmarking::WorkloadProfile profile_;
};

// Test WorkloadProfile::IsValid() method
TEST_F(WorkloadGeneratorTest, ProfileValidation) {
    // Valid profile
    EXPECT_TRUE(profile_.IsValid());
    
    // Invalid profiles
    
    // Zero total size
    auto invalid_profile = profile_;
    invalid_profile.total_size = 0;
    EXPECT_FALSE(invalid_profile.IsValid());
    
    // Zero block size
    invalid_profile = profile_;
    invalid_profile.block_size = 0;
    EXPECT_FALSE(invalid_profile.IsValid());
    
    // Zero num blocks
    invalid_profile = profile_;
    invalid_profile.num_blocks = 0;
    EXPECT_FALSE(invalid_profile.IsValid());
    
    // Read + write != 100%
    invalid_profile = profile_;
    invalid_profile.read_percentage = 60;
    invalid_profile.write_percentage = 60;
    EXPECT_FALSE(invalid_profile.IsValid());
    
    // Random > 100%
    invalid_profile = profile_;
    invalid_profile.random_percentage = 110;
    EXPECT_FALSE(invalid_profile.IsValid());
}

// Test WorkloadGenerator constructor with invalid parameters
TEST_F(WorkloadGeneratorTest, ConstructorInvalidParams) {
    // Null controller
    EXPECT_THROW(
        nvmeof::benchmarking::WorkloadGenerator(nullptr, reinterpret_cast<const spdk_nvme_qpair*>(1), profile_),
        std::invalid_argument
    );
    
    // Null queue pair
    EXPECT_THROW(
        nvmeof::benchmarking::WorkloadGenerator(reinterpret_cast<const spdk_nvme_ctrlr*>(1), nullptr, profile_),
        std::invalid_argument
    );
    
    // Invalid profile
    auto invalid_profile = profile_;
    invalid_profile.total_size = 0;
    EXPECT_THROW(
        nvmeof::benchmarking::WorkloadGenerator(
            reinterpret_cast<const spdk_nvme_ctrlr*>(1),
            reinterpret_cast<const spdk_nvme_qpair*>(1),
            invalid_profile
        ),
        std::invalid_argument
    );
}

// Test GetProgress method
TEST_F(WorkloadGeneratorTest, GetProgress) {
    // This is a partial test since we can't actually run the generator without real NVMe hardware
    // We can test some basic functionality though
    
    // Create a generator with mock objects
    auto generator = nvmeof::benchmarking::WorkloadGenerator(
        reinterpret_cast<const spdk_nvme_ctrlr*>(1),
        reinterpret_cast<const spdk_nvme_qpair*>(1),
        profile_
    );
    
    // Initial progress should be 0
    EXPECT_DOUBLE_EQ(0.0, generator.GetProgress());
    
    // We'd need to mock the internal state to test progress updating
}

// Test Stop method
TEST_F(WorkloadGeneratorTest, Stop) {
    // Create a generator with mock objects
    auto generator = nvmeof::benchmarking::WorkloadGenerator(
        reinterpret_cast<const spdk_nvme_ctrlr*>(1),
        reinterpret_cast<const spdk_nvme_qpair*>(1),
        profile_
    );
    
    // Stop should not throw
    EXPECT_NO_THROW(generator.Stop());
    
    // After stopping, progress should still be 0
    EXPECT_DOUBLE_EQ(0.0, generator.GetProgress());
}

// Test callback functionality
TEST_F(WorkloadGeneratorTest, Callback) {
    bool callback_called = false;
    uint32_t callback_bytes = 0;
    
    // Create a callback
    auto callback = [&callback_called, &callback_bytes](bool /*success*/, uint32_t bytes_processed) {
        callback_called = true;
        callback_bytes = bytes_processed;
    };
    
    // Create a generator with the callback
    auto generator = nvmeof::benchmarking::WorkloadGenerator(
        reinterpret_cast<const spdk_nvme_ctrlr*>(1),
        reinterpret_cast<const spdk_nvme_qpair*>(1),
        profile_,
        callback
    );
    
    // We can't really test the callback without running the generator
    // This would require more sophisticated mocking of the SPDK APIs
}

// Additional tests would be implemented for real hardware or with more sophisticated mocking