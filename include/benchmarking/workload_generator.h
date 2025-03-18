#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <functional>
// #include <spdk/nvme.h>
#include "../../third_party/spdk_mock/include/nvme.h"
#include <cassert>

namespace nvmeof {
namespace benchmarking {

/**
 * @brief Defines the characteristics of a workload to be generated.
 * 
 * This structure contains parameters that define the workload profile,
 * such as total size, block size, number of blocks, and interval.
 */
struct WorkloadProfile {
    uint64_t total_size;       ///< Total size of the workload in bytes
    uint32_t block_size;       ///< Size of each block in bytes
    uint32_t num_blocks;       ///< Number of blocks to be generated
    uint32_t interval_us;      ///< Interval between operations in microseconds
    uint32_t read_percentage;  ///< Percentage of read operations (0-100)
    uint32_t write_percentage; ///< Percentage of write operations (0-100)
    uint32_t random_percentage; ///< Percentage of random operations (0-100)
    
    /**
     * @brief Validates the workload profile parameters.
     * 
     * @return true if the profile is valid, false otherwise.
     */
    bool IsValid() const {
        // Basic validation
        return (total_size > 0 && 
                block_size > 0 && 
                num_blocks > 0 && 
                read_percentage + write_percentage == 100 &&
                random_percentage <= 100);
    }
};

/**
 * @brief Callback type for I/O completion notifications.
 */
using IoCompletionCallback = std::function<void(bool success, uint32_t bytes_processed)>;

/**
 * @brief Generator for NVMe-oF benchmarking workloads.
 * 
 * This class is responsible for generating workloads based on a specified profile
 * and executing them against an NVMe controller.
 */
class WorkloadGenerator {
public:
    /**
     * @brief Constructs a WorkloadGenerator with the specified controller, queue pair, and profile.
     * 
     * @param ctrlr Pointer to the NVMe controller
     * @param qpair Pointer to the NVMe queue pair
     * @param profile Workload profile defining the characteristics of the workload
     * @param completion_callback Optional callback to be invoked upon workload completion
     * 
     * @throws std::invalid_argument If the profile is invalid or the controller/queue pair is null
     */
    WorkloadGenerator(const struct spdk_nvme_ctrlr *ctrlr, 
                      const struct spdk_nvme_qpair *qpair,
                      const WorkloadProfile& profile,
                      IoCompletionCallback completion_callback = nullptr);

    /**
     * @brief Destroys the WorkloadGenerator, cleaning up any allocated resources.
     */
    ~WorkloadGenerator();

    /**
     * @brief Generates and executes the workload according to the specified profile.
     * 
     * @return true if the workload was generated and executed successfully, false otherwise.
     * 
     * @throws std::runtime_error If the workload generation fails
     */
    bool Generate();

    /**
     * @brief Stops the workload generation if it's in progress.
     */
    void Stop();

    /**
     * @brief Gets the current progress of the workload generation.
     * 
     * @return A value between 0.0 and 1.0 indicating the progress.
     */
    double GetProgress() const;

private:
    /**
     * @brief Writes a block of data to the NVMe device.
     * 
     * @param offset Offset in bytes where the write should start
     * @param size Size of the block to write in bytes
     * 
     * @return true if the write operation was submitted successfully, false otherwise.
     */
    bool WriteBlock(uint64_t offset, uint32_t size);

    /**
     * @brief Reads a block of data from the NVMe device.
     * 
     * @param offset Offset in bytes where the read should start
     * @param size Size of the block to read in bytes
     * 
     * @return true if the read operation was submitted successfully, false otherwise.
     */
    bool ReadBlock(uint64_t offset, uint32_t size);

    /**
     * @brief Callback function for write completion.
     * 
     * @param arg User-provided argument (this WorkloadGenerator instance)
     * @param completion The NVMe completion structure
     */
    static void WriteCompletionCallback(void *arg, const struct spdk_nvme_cpl *completion);

    /**
     * @brief Callback function for read completion.
     * 
     * @param arg User-provided argument (this WorkloadGenerator instance)
     * @param completion The NVMe completion structure
     */
    static void ReadCompletionCallback(void *arg, const struct spdk_nvme_cpl *completion);

    // NVMe controller and queue pair
    const struct spdk_nvme_ctrlr *ctrlr_;
    const struct spdk_nvme_qpair *qpair_;
    
    // Workload profile and state
    WorkloadProfile profile_;
    bool write_completed_;
    bool read_completed_;
    uint64_t total_bytes_processed_;
    bool is_running_;
    
    // Completion callback
    IoCompletionCallback completion_callback_;
};

}  // namespace benchmarking
}  // namespace nvmeof