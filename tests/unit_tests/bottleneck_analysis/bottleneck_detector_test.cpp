#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../../include/bottleneck_analysis/bottleneck_detector.h"
#include "../../../include/bottleneck_analysis/resource_monitor.h"

using namespace nvmeof::bottleneck_analysis;

// Mock for ResourceUsage
class MockResourceUsage : public ResourceUsage {
public:
    MockResourceUsage() {
        // Set default values
        cpu_usage_percent = 50.0;
        total_memory_bytes = 16 * 1024 * 1024 * 1024ULL; // 16 GB
        used_memory_bytes = 8 * 1024 * 1024 * 1024ULL;   // 8 GB
        memory_usage_percent = 50.0;
        
        // Add a mock network interface
        interfaces.push_back("eth0");
        rx_bytes.push_back(5000000);
        tx_bytes.push_back(5000000);
        rx_packets.push_back(10000);
        tx_packets.push_back(10000);
        
        timestamp = std::chrono::system_clock::now();
    }
};

// Test fixture for BottleneckDetector tests
class BottleneckDetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create default thresholds
        cpu_threshold_ = 80.0;
        memory_threshold_ = 90.0;
        network_threshold_ = 1000000000; // 1 GB/s
        storage_threshold_ = 500000000;  // 500 MB/s
        
        // Create a detector with default thresholds
        detector_ = std::make_unique<BottleneckDetector>(
            cpu_threshold_,
            memory_threshold_,
            network_threshold_,
            storage_threshold_
        );
    }
    
    double cpu_threshold_;
    double memory_threshold_;
    uint64_t network_threshold_;
    uint64_t storage_threshold_;
    std::unique_ptr<BottleneckDetector> detector_;
};

// Test BottleneckInfo constructor with invalid severity
TEST_F(BottleneckDetectorTest, BottleneckInfoInvalidSeverity) {
    // Too low
    EXPECT_THROW(
        BottleneckInfo(BottleneckType::CPU, "Test", -0.1, "CPU", 90.0, "Test recommendation"),
        std::invalid_argument
    );
    
    // Too high
    EXPECT_THROW(
        BottleneckInfo(BottleneckType::CPU, "Test", 1.1, "CPU", 90.0, "Test recommendation"),
        std::invalid_argument
    );
    
    // Valid
    EXPECT_NO_THROW(
        BottleneckInfo(BottleneckType::CPU, "Test", 0.5, "CPU", 90.0, "Test recommendation")
    );
}

// Test BottleneckDetector constructor with invalid thresholds
TEST_F(BottleneckDetectorTest, ConstructorInvalidThresholds) {
    // Invalid CPU threshold (negative)
    EXPECT_THROW(
        BottleneckDetector(-10.0, memory_threshold_, network_threshold_, storage_threshold_),
        std::invalid_argument
    );
    
    // Invalid CPU threshold (over 100%)
    EXPECT_THROW(
        BottleneckDetector(110.0, memory_threshold_, network_threshold_, storage_threshold_),
        std::invalid_argument
    );
    
    // Invalid memory threshold
    EXPECT_THROW(
        BottleneckDetector(cpu_threshold_, 150.0, network_threshold_, storage_threshold_),
        std::invalid_argument
    );
    
    // Invalid network threshold (zero)
    EXPECT_THROW(
        BottleneckDetector(cpu_threshold_, memory_threshold_, 0, storage_threshold_),
        std::invalid_argument
    );
    
    // Invalid storage threshold (zero)
    EXPECT_THROW(
        BottleneckDetector(cpu_threshold_, memory_threshold_, network_threshold_, 0),
        std::invalid_argument
    );
}

// Test setting thresholds with invalid values
TEST_F(BottleneckDetectorTest, SetInvalidThresholds) {
    // CPU threshold
    EXPECT_THROW(detector_->SetCpuThreshold(-10.0), std::invalid_argument);
    EXPECT_THROW(detector_->SetCpuThreshold(110.0), std::invalid_argument);
    
    // Memory threshold
    EXPECT_THROW(detector_->SetMemoryThreshold(-10.0), std::invalid_argument);
    EXPECT_THROW(detector_->SetMemoryThreshold(110.0), std::invalid_argument);
    
    // Network threshold
    EXPECT_THROW(detector_->SetNetworkThreshold(0), std::invalid_argument);
    
    // Storage threshold
    EXPECT_THROW(detector_->SetStorageThreshold(0), std::invalid_argument);
}

// Test detecting no bottlenecks
TEST_F(BottleneckDetectorTest, DetectNoBottlenecks) {
    // All usage is below thresholds
    double cpu_usage = cpu_threshold_ - 10.0;
    double memory_usage = memory_threshold_ - 10.0;
    uint64_t network_usage = network_threshold_ - 100000000;
    uint64_t storage_usage = storage_threshold_ - 100000000;
    
    // Detect bottlenecks
    auto bottlenecks = detector_->DetectBottlenecks(cpu_usage, memory_usage, network_usage, storage_usage);
    
    // Expect empty vector (no bottlenecks)
    EXPECT_TRUE(bottlenecks.empty());
}

// Test detecting CPU bottleneck
TEST_F(BottleneckDetectorTest, DetectCpuBottleneck) {
    // CPU usage above threshold
    double cpu_usage = cpu_threshold_ + 10.0;
    double memory_usage = memory_threshold_ - 10.0;
    uint64_t network_usage = network_threshold_ - 100000000;
    uint64_t storage_usage = storage_threshold_ - 100000000;
    
    // Detect bottlenecks
    auto bottlenecks = detector_->DetectBottlenecks(cpu_usage, memory_usage, network_usage, storage_usage);
    
    // Expect one bottleneck
    ASSERT_EQ(1, bottlenecks.size());
    
    // Check bottleneck properties
    EXPECT_EQ(BottleneckType::CPU, bottlenecks[0].type);
    EXPECT_EQ("CPU", bottlenecks[0].resource_name);
    EXPECT_DOUBLE_EQ(cpu_usage, bottlenecks[0].resource_usage);
    EXPECT_GT(bottlenecks[0].severity, 0.0);
    EXPECT_LE(bottlenecks[0].severity, 1.0);
}

// Test detecting memory bottleneck
TEST_F(BottleneckDetectorTest, DetectMemoryBottleneck) {
    // Memory usage above threshold
    double cpu_usage = cpu_threshold_ - 10.0;
    double memory_usage = memory_threshold_ + 5.0;
    uint64_t network_usage = network_threshold_ - 100000000;
    uint64_t storage_usage = storage_threshold_ - 100000000;
    
    // Detect bottlenecks
    auto bottlenecks = detector_->DetectBottlenecks(cpu_usage, memory_usage, network_usage, storage_usage);
    
    // Expect one bottleneck
    ASSERT_EQ(1, bottlenecks.size());
    
    // Check bottleneck properties
    EXPECT_EQ(BottleneckType::MEMORY, bottlenecks[0].type);
    EXPECT_EQ("Memory", bottlenecks[0].resource_name);
    EXPECT_DOUBLE_EQ(memory_usage, bottlenecks[0].resource_usage);
}

// Test detecting network bottleneck
TEST_F(BottleneckDetectorTest, DetectNetworkBottleneck) {
    // Network usage above threshold
    double cpu_usage = cpu_threshold_ - 10.0;
    double memory_usage = memory_threshold_ - 10.0;
    uint64_t network_usage = network_threshold_ + 500000000;
    uint64_t storage_usage = storage_threshold_ - 100000000;
    
    // Detect bottlenecks
    auto bottlenecks = detector_->DetectBottlenecks(cpu_usage, memory_usage, network_usage, storage_usage);
    
    // Expect one bottleneck
    ASSERT_EQ(1, bottlenecks.size());
    
    // Check bottleneck properties
    EXPECT_EQ(BottleneckType::NETWORK, bottlenecks[0].type);
    EXPECT_EQ("Network", bottlenecks[0].resource_name);
    EXPECT_DOUBLE_EQ(static_cast<double>(network_usage), bottlenecks[0].resource_usage);
}

// Test detecting storage bottleneck
TEST_F(BottleneckDetectorTest, DetectStorageBottleneck) {
    // Storage usage above threshold
    double cpu_usage = cpu_threshold_ - 10.0;
    double memory_usage = memory_threshold_ - 10.0;
    uint64_t network_usage = network_threshold_ - 100000000;
    uint64_t storage_usage = storage_threshold_ + 100000000;
    
    // Detect bottlenecks
    auto bottlenecks = detector_->DetectBottlenecks(cpu_usage, memory_usage, network_usage, storage_usage);
    
    // Expect one bottleneck
    ASSERT_EQ(1, bottlenecks.size());
    
    // Check bottleneck properties
    EXPECT_EQ(BottleneckType::STORAGE, bottlenecks[0].type);
    EXPECT_EQ("Storage", bottlenecks[0].resource_name);
    EXPECT_DOUBLE_EQ(static_cast<double>(storage_usage), bottlenecks[0].resource_usage);
}

// Test detecting multiple bottlenecks
TEST_F(BottleneckDetectorTest, DetectMultipleBottlenecks) {
    // Multiple usage metrics above thresholds
    double cpu_usage = cpu_threshold_ + 10.0;
    double memory_usage = memory_threshold_ + 5.0;
    uint64_t network_usage = network_threshold_ + 500000000;
    uint64_t storage_usage = storage_threshold_ - 100000000; // Below threshold
    
    // Detect bottlenecks
    auto bottlenecks = detector_->DetectBottlenecks(cpu_usage, memory_usage, network_usage, storage_usage);
    
    // Expect three bottlenecks
    ASSERT_EQ(3, bottlenecks.size());
    
    // Check bottleneck types (not necessarily in this order)
    std::set<BottleneckType> bottleneck_types;
    for (const auto& bottleneck : bottlenecks) {
        bottleneck_types.insert(bottleneck.type);
    }
    
    EXPECT_TRUE(bottleneck_types.find(BottleneckType::CPU) != bottleneck_types.end());
    EXPECT_TRUE(bottleneck_types.find(BottleneckType::MEMORY) != bottleneck_types.end());
    EXPECT_TRUE(bottleneck_types.find(BottleneckType::NETWORK) != bottleneck_types.end());
    EXPECT_TRUE(bottleneck_types.find(BottleneckType::STORAGE) == bottleneck_types.end());
}

// Test with ResourceUsage structure
TEST_F(BottleneckDetectorTest, DetectWithResourceUsage) {
    // Create a custom resource usage
    MockResourceUsage usage;
    
    // Set values that should trigger bottlenecks
    usage.cpu_usage_percent = cpu_threshold_ + 10.0;
    usage.memory_usage_percent = memory_threshold_ - 10.0; // Below threshold
    usage.rx_bytes[0] = network_threshold_; // At threshold
    usage.tx_bytes[0] = 100000000; // Extra to push over threshold
    
    // Detect bottlenecks
    auto bottlenecks = detector_->DetectBottlenecks(usage);
    
    // Expect bottlenecks for CPU and network
    ASSERT_EQ(2, bottlenecks.size());
    
    // Check bottleneck types (not necessarily in this order)
    std::set<BottleneckType> bottleneck_types;
    for (const auto& bottleneck : bottlenecks) {
        bottleneck_types.insert(bottleneck.type);
    }
    
    EXPECT_TRUE(bottleneck_types.find(BottleneckType::CPU) != bottleneck_types.end());
    EXPECT_TRUE(bottleneck_types.find(BottleneckType::NETWORK) != bottleneck_types.end());
    EXPECT_TRUE(bottleneck_types.find(BottleneckType::MEMORY) == bottleneck_types.end());
}

// Test callback functionality
TEST_F(BottleneckDetectorTest, CallbackFunctionality) {
    // Create a vector to track callback calls
    std::vector<BottleneckType> detected_types;
    
    // Set up callback
    detector_->SetCallback([&detected_types](const BottleneckInfo& info) {
        detected_types.push_back(info.type);
    });
    
    // Multiple usage metrics above thresholds
    double cpu_usage = cpu_threshold_ + 10.0;
    double memory_usage = memory_threshold_ + 5.0;
    uint64_t network_usage = network_threshold_ - 100000000; // Below threshold
    uint64_t storage_usage = storage_threshold_ + 100000000;
    
    // Detect bottlenecks
    detector_->DetectBottlenecks(cpu_usage, memory_usage, network_usage, storage_usage);
    
    // Expect callbacks for CPU, memory, and storage bottlenecks
    ASSERT_EQ(3, detected_types.size());
    
    // Check bottleneck types (not necessarily in this order)
    std::set<BottleneckType> bottleneck_types(detected_types.begin(), detected_types.end());
    
    EXPECT_TRUE(bottleneck_types.find(BottleneckType::CPU) != bottleneck_types.end());
    EXPECT_TRUE(bottleneck_types.find(BottleneckType::MEMORY) != bottleneck_types.end());
    EXPECT_TRUE(bottleneck_types.find(BottleneckType::STORAGE) != bottleneck_types.end());
    EXPECT_TRUE(bottleneck_types.find(BottleneckType::NETWORK) == bottleneck_types.end());
}