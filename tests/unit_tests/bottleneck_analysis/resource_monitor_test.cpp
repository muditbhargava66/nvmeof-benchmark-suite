#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../../include/bottleneck_analysis/resource_monitor.h"
#include <thread>
#include <chrono>
#include <atomic>

using namespace nvmeof::bottleneck_analysis;

class ResourceMonitorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Default monitor interval
        monitor_interval_ = std::chrono::milliseconds(100);
    }
    
    std::chrono::milliseconds monitor_interval_;
};

// Test ResourceUsage constructor
TEST_F(ResourceMonitorTest, ResourceUsageConstructor) {
    ResourceUsage usage;
    
    // Default values should be initialized
    EXPECT_EQ(0.0, usage.cpu_usage_percent);
    EXPECT_EQ(0UL, usage.total_memory_bytes);
    EXPECT_EQ(0UL, usage.used_memory_bytes);
    EXPECT_EQ(0.0, usage.memory_usage_percent);
    EXPECT_TRUE(usage.interfaces.empty());
    EXPECT_TRUE(usage.rx_bytes.empty());
    EXPECT_TRUE(usage.tx_bytes.empty());
    EXPECT_TRUE(usage.rx_packets.empty());
    EXPECT_TRUE(usage.tx_packets.empty());
    
    // Timestamp should be initialized to a valid time
    auto now = std::chrono::system_clock::now();
    auto diff = now - usage.timestamp;
    auto diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
    
    // Timestamp should be within the last 5 seconds (arbitrary reasonable value)
    EXPECT_LT(diff_ms, 5000);
}

// Test ResourceUsage::GetMemoryUsagePercent method
TEST_F(ResourceMonitorTest, GetMemoryUsagePercent) {
    ResourceUsage usage;
    
    // Test with zero total memory (should return 0)
    usage.total_memory_bytes = 0;
    usage.used_memory_bytes = 0;
    EXPECT_DOUBLE_EQ(0.0, usage.GetMemoryUsagePercent());
    
    // Test with non-zero total memory
    usage.total_memory_bytes = 100;
    usage.used_memory_bytes = 50;
    EXPECT_DOUBLE_EQ(50.0, usage.GetMemoryUsagePercent());
    
    // Test with used > total (should cap at 100%)
    usage.total_memory_bytes = 100;
    usage.used_memory_bytes = 150;
    EXPECT_DOUBLE_EQ(100.0, usage.GetMemoryUsagePercent());
}

// Test ResourceMonitor constructor
TEST_F(ResourceMonitorTest, Constructor) {
    // Basic constructor (no callback)
    ResourceMonitor monitor(monitor_interval_);
    
    // Monitor should not be running initially
    EXPECT_FALSE(monitor.IsRunning());
    
    // Interval should be set correctly
    EXPECT_EQ(monitor_interval_, monitor.GetInterval());
    
    // Test constructor with callback
    bool callback_called = false;
    auto callback = [&callback_called](const ResourceUsage& /*usage*/) {
        callback_called = true;
    };
    
    ResourceMonitor monitor_with_callback(monitor_interval_, callback);
    
    // Monitor should not be running initially
    EXPECT_FALSE(monitor_with_callback.IsRunning());
    
    // Callback should not be called yet
    EXPECT_FALSE(callback_called);
}

// Test constructor with invalid interval
TEST_F(ResourceMonitorTest, ConstructorInvalidInterval) {
    // Zero interval should throw
    EXPECT_THROW(
        ResourceMonitor monitor(std::chrono::milliseconds(0)),
        std::invalid_argument
    );
}

// Test Start and Stop methods
TEST_F(ResourceMonitorTest, StartStop) {
    ResourceMonitor monitor(monitor_interval_);
    
    // Start the monitor
    EXPECT_TRUE(monitor.Start());
    
    // Monitor should be running
    EXPECT_TRUE(monitor.IsRunning());
    
    // Stop the monitor
    EXPECT_TRUE(monitor.Stop());
    
    // Monitor should not be running
    EXPECT_FALSE(monitor.IsRunning());
}

// Test starting an already running monitor
TEST_F(ResourceMonitorTest, StartAlreadyRunning) {
    ResourceMonitor monitor(monitor_interval_);
    
    // Start the monitor
    EXPECT_TRUE(monitor.Start());
    
    // Try to start again (should throw or return false)
    EXPECT_THROW(monitor.Start(), std::runtime_error);
    
    // Stop the monitor
    EXPECT_TRUE(monitor.Stop());
}

// Test stopping a non-running monitor
TEST_F(ResourceMonitorTest, StopNotRunning) {
    ResourceMonitor monitor(monitor_interval_);
    
    // Try to stop the monitor (should return false)
    EXPECT_FALSE(monitor.Stop());
}

// Test callback functionality
TEST_F(ResourceMonitorTest, Callback) {
    std::atomic<int> callback_count{0};
    auto callback = [&callback_count](const ResourceUsage& /*usage*/) {
        callback_count++;
    };
    
    ResourceMonitor monitor(monitor_interval_, callback);
    
    // Start the monitor
    EXPECT_TRUE(monitor.Start());
    
    // Wait for callbacks to be called
    std::this_thread::sleep_for(monitor_interval_ * 5);
    
    // Stop the monitor
    EXPECT_TRUE(monitor.Stop());
    
    // Callback should have been called at least once
    EXPECT_GT(callback_count, 0);
    
    // Save the current count
    int current_count = callback_count;
    
    // Wait again to ensure no more callbacks are called
    std::this_thread::sleep_for(monitor_interval_ * 2);
    
    // Count should not have changed
    EXPECT_EQ(current_count, callback_count);
}

// Test GetLatestUsage method
TEST_F(ResourceMonitorTest, GetLatestUsage) {
    ResourceMonitor monitor(monitor_interval_);
    
    // Start the monitor
    EXPECT_TRUE(monitor.Start());
    
    // Wait for some data to be collected
    std::this_thread::sleep_for(monitor_interval_ * 2);
    
    // Get the latest usage
    ResourceUsage usage = monitor.GetLatestUsage();
    
    // Stop the monitor
    EXPECT_TRUE(monitor.Stop());
    
    // Check that the usage data is reasonable
    
    // CPU usage should be between 0 and 100%
    EXPECT_GE(usage.cpu_usage_percent, 0.0);
    EXPECT_LE(usage.cpu_usage_percent, 100.0);
    
    // Total memory should be greater than zero
    EXPECT_GT(usage.total_memory_bytes, 0UL);
    
    // Used memory should be less than or equal to total memory
    EXPECT_LE(usage.used_memory_bytes, usage.total_memory_bytes);
    
    // Memory percentage should be between 0 and 100%
    double memory_percent = usage.GetMemoryUsagePercent();
    EXPECT_GE(memory_percent, 0.0);
    EXPECT_LE(memory_percent, 100.0);
    
    // There should be at least one network interface
    EXPECT_FALSE(usage.interfaces.empty());
    
    // Network stats arrays should have the same size as interfaces
    EXPECT_EQ(usage.interfaces.size(), usage.rx_bytes.size());
    EXPECT_EQ(usage.interfaces.size(), usage.tx_bytes.size());
    EXPECT_EQ(usage.interfaces.size(), usage.rx_packets.size());
    EXPECT_EQ(usage.interfaces.size(), usage.tx_packets.size());
    
    // Timestamp should be reasonable (within the last few seconds)
    auto now = std::chrono::system_clock::now();
    auto diff = now - usage.timestamp;
    auto diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
    EXPECT_LT(diff_ms, 5000);
}

// Test SetInterval method
TEST_F(ResourceMonitorTest, SetInterval) {
    ResourceMonitor monitor(monitor_interval_);
    
    // Change the interval
    std::chrono::milliseconds new_interval(200);
    monitor.SetInterval(new_interval);
    
    // Check that the interval was changed
    EXPECT_EQ(new_interval, monitor.GetInterval());
    
    // Test with invalid interval
    EXPECT_THROW(
        monitor.SetInterval(std::chrono::milliseconds(0)),
        std::invalid_argument
    );
}

// Test SetCallback method
TEST_F(ResourceMonitorTest, SetCallback) {
    ResourceMonitor monitor(monitor_interval_);
    
    // Start without a callback
    EXPECT_TRUE(monitor.Start());
    
    // Wait for some data to be collected
    std::this_thread::sleep_for(monitor_interval_ * 2);
    
    // Set a callback
    std::atomic<int> callback_count{0};
    auto callback = [&callback_count](const ResourceUsage& /*usage*/) {
        callback_count++;
    };
    
    monitor.SetCallback(callback);
    
    // Wait for the callback to be called
    std::this_thread::sleep_for(monitor_interval_ * 5);
    
    // Stop the monitor
    EXPECT_TRUE(monitor.Stop());
    
    // Callback should have been called at least once
    EXPECT_GT(callback_count, 0);
}

// Test that ResourceMonitor doesn't support move operations due to std::atomic member
TEST_F(ResourceMonitorTest, MoveOperationsNotSupported) {
    // Create a monitor
    ResourceMonitor monitor1(monitor_interval_);
    
    // Verify that ResourceMonitor cannot be copied or moved
    // due to the std::atomic<bool> member
    static_assert(!std::is_copy_constructible<ResourceMonitor>::value, "ResourceMonitor should not be copy constructible");
    static_assert(!std::is_move_constructible<ResourceMonitor>::value, "ResourceMonitor should not be move constructible");
    static_assert(!std::is_copy_assignable<ResourceMonitor>::value, "ResourceMonitor should not be copy assignable");
    static_assert(!std::is_move_assignable<ResourceMonitor>::value, "ResourceMonitor should not be move assignable");
    
    // Start and stop the monitor to verify it still works
    EXPECT_TRUE(monitor1.Start());
    EXPECT_TRUE(monitor1.Stop());
}