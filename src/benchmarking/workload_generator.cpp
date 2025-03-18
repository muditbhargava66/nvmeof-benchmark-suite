#include "../../include/benchmarking/workload_generator.h"
#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <cassert>
#include <algorithm>

namespace nvmeof {
namespace benchmarking {

WorkloadGenerator::WorkloadGenerator(const struct spdk_nvme_ctrlr *ctrlr, 
                                    const struct spdk_nvme_qpair *qpair,
                                    const WorkloadProfile& profile,
                                    IoCompletionCallback completion_callback)
    : ctrlr_(ctrlr)
    , qpair_(qpair)
    , profile_(profile)
    , write_completed_(true)
    , read_completed_(true)
    , total_bytes_processed_(0)
    , is_running_(false)
    , completion_callback_(completion_callback) {
    
    // Validate parameters
    if (ctrlr_ == nullptr) {
        throw std::invalid_argument("NVMe controller cannot be null");
    }
    
    if (qpair_ == nullptr) {
        throw std::invalid_argument("NVMe queue pair cannot be null");
    }
    
    if (!profile_.IsValid()) {
        throw std::invalid_argument("Invalid workload profile");
    }
}

WorkloadGenerator::~WorkloadGenerator() {
    // Ensure workload generation is stopped before destruction
    Stop();
}

bool WorkloadGenerator::Generate() {
    // Check if already running
    if (is_running_) {
        std::cerr << "Warning: Workload generation is already in progress" << std::endl;
        return false;
    }
    
    is_running_ = true;
    total_bytes_processed_ = 0;
    
    // Set up random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, profile_.num_blocks - 1);
    std::uniform_int_distribution<uint32_t> op_dist(1, 100); // For determining read vs write
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        // Main workload generation loop
        while (is_running_ && total_bytes_processed_ < profile_.total_size) {
            // Determine block offset based on randomness percentage
            uint64_t block_index;
            if (static_cast<uint32_t>(op_dist(gen)) <= profile_.random_percentage) {
                // Random access
                block_index = dist(gen);
            } else {
                // Sequential access
                block_index = (total_bytes_processed_ / profile_.block_size) % profile_.num_blocks;
            }
            
            uint64_t block_offset = block_index * profile_.block_size;
            uint32_t block_size = std::min(profile_.block_size, 
                                        static_cast<uint32_t>(profile_.total_size - total_bytes_processed_));
            
            // Determine operation type (read or write)
            bool is_read = static_cast<uint32_t>(op_dist(gen)) <= profile_.read_percentage;
            
            bool success = false;
            if (is_read) {
                success = ReadBlock(block_offset, block_size);
            } else {
                success = WriteBlock(block_offset, block_size);
            }
            
            if (!success) {
                std::cerr << "Error: " << (is_read ? "Read" : "Write") << " operation failed at offset " 
                         << block_offset << " with size " << block_size << std::endl;
                continue;
            }
            
            total_bytes_processed_ += block_size;
            
            // Sleep for the specified interval
            std::this_thread::sleep_for(std::chrono::microseconds(profile_.interval_us));
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;
        
        // Log completion and statistics
        std::cout << "Workload generation " 
                 << (is_running_ ? "completed" : "stopped") << "." << std::endl;
        std::cout << "Total bytes processed: " << total_bytes_processed_ << std::endl;
        std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;
        
        // Notify completion if callback is provided
        if (completion_callback_) {
            completion_callback_(true, total_bytes_processed_);
        }
        
        is_running_ = false;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error during workload generation: " << e.what() << std::endl;
        is_running_ = false;
        
        // Notify failure if callback is provided
        if (completion_callback_) {
            completion_callback_(false, total_bytes_processed_);
        }
        
        return false;
    }
}

void WorkloadGenerator::Stop() {
    is_running_ = false;
}

double WorkloadGenerator::GetProgress() const {
    if (profile_.total_size == 0) {
        return 0.0;
    }
    
    return static_cast<double>(total_bytes_processed_) / profile_.total_size;
}

bool WorkloadGenerator::WriteBlock(uint64_t offset, uint32_t size) {
    assert(ctrlr_ != nullptr);
    assert(qpair_ != nullptr);
    
    struct spdk_nvme_ns *ns = const_cast<struct spdk_nvme_ns*>(
        spdk_nvme_ctrlr_get_ns(ctrlr_, 1)
    );
    
    if (ns == nullptr) {
        std::cerr << "Error: Namespace not found" << std::endl;
        return false;
    }
    
    // Get the namespace sector size
    uint32_t sector_size = spdk_nvme_ns_get_sector_size(ns);
    if (sector_size == 0) {
        std::cerr << "Error: Invalid sector size" << std::endl;
        return false;
    }
    
    // Ensure size is aligned to sector size
    if (size % sector_size != 0) {
        size = (size / sector_size + 1) * sector_size;
    }
    
    // Allocate a buffer for writing data
    void *buffer = spdk_dma_malloc(size, 0, nullptr);
    if (buffer == nullptr) {
        std::cerr << "Error: Memory allocation failed for write buffer" << std::endl;
        return false;
    }
    
    // Fill the buffer with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 255);
    uint8_t *data = static_cast<uint8_t *>(buffer);
    for (uint32_t i = 0; i < size; ++i) {
        data[i] = dist(gen);
    }
    
    // Calculate LBA and LBA count
    uint64_t lba = offset / sector_size;
    uint32_t lba_count = size / sector_size;
    
    // Reset completion flag before submission
    write_completed_ = false;
    
    // Need to remove const qualifier for the mock SPDK library
    struct spdk_nvme_qpair* non_const_qpair = const_cast<struct spdk_nvme_qpair*>(qpair_);
    
    // Submit the write operation
    int rc = spdk_nvme_ns_cmd_write(ns, non_const_qpair, buffer, lba, lba_count, 
                                    WriteCompletionCallback, this, 0);
    if (rc != 0) {
        std::cerr << "Error: Failed to submit write command, rc=" << rc << std::endl;
        spdk_dma_free(buffer);
        return false;
    }
    
    // Poll for completion
    while (!write_completed_) {
        spdk_nvme_qpair_process_completions(non_const_qpair, 0);
    }
    
    // Free the buffer
    spdk_dma_free(buffer);
    
    return true;
}

bool WorkloadGenerator::ReadBlock(uint64_t offset, uint32_t size) {
    assert(ctrlr_ != nullptr);
    assert(qpair_ != nullptr);
    
    struct spdk_nvme_ns *ns = const_cast<struct spdk_nvme_ns*>(
        spdk_nvme_ctrlr_get_ns(ctrlr_, 1)
    );
    
    if (ns == nullptr) {
        std::cerr << "Error: Namespace not found" << std::endl;
        return false;
    }
    
    // Get the namespace sector size
    uint32_t sector_size = spdk_nvme_ns_get_sector_size(ns);
    if (sector_size == 0) {
        std::cerr << "Error: Invalid sector size" << std::endl;
        return false;
    }
    
    // Ensure size is aligned to sector size
    if (size % sector_size != 0) {
        size = (size / sector_size + 1) * sector_size;
    }
    
    // Allocate a buffer for reading data
    void *buffer = spdk_dma_malloc(size, 0, nullptr);
    if (buffer == nullptr) {
        std::cerr << "Error: Memory allocation failed for read buffer" << std::endl;
        return false;
    }
    
    // Calculate LBA and LBA count
    uint64_t lba = offset / sector_size;
    uint32_t lba_count = size / sector_size;
    
    // Reset completion flag before submission
    read_completed_ = false;
    
    // Need to remove const qualifier for the mock SPDK library
    struct spdk_nvme_qpair* non_const_qpair = const_cast<struct spdk_nvme_qpair*>(qpair_);
    
    // Submit the read operation
    int rc = spdk_nvme_ns_cmd_read(ns, non_const_qpair, buffer, lba, lba_count, 
                                    ReadCompletionCallback, this, 0);
    if (rc != 0) {
        std::cerr << "Error: Failed to submit read command, rc=" << rc << std::endl;
        spdk_dma_free(buffer);
        return false;
    }
    
    // Poll for completion
    while (!read_completed_) {
        spdk_nvme_qpair_process_completions(non_const_qpair, 0);
    }
    
    // Free the buffer
    spdk_dma_free(buffer);
    
    return true;
}

void WorkloadGenerator::WriteCompletionCallback(void *arg, const struct spdk_nvme_cpl *completion) {
    auto generator = static_cast<WorkloadGenerator*>(arg);
    assert(generator != nullptr);
    
    if (spdk_nvme_cpl_is_error(completion)) {
        std::cerr << "Error: Write operation failed with status code: " 
                 << static_cast<int>(completion->status.sc) << std::endl;
    }
    
    generator->write_completed_ = true;
}

void WorkloadGenerator::ReadCompletionCallback(void *arg, const struct spdk_nvme_cpl *completion) {
    auto generator = static_cast<WorkloadGenerator*>(arg);
    assert(generator != nullptr);
    
    if (spdk_nvme_cpl_is_error(completion)) {
        std::cerr << "Error: Read operation failed with status code: " 
                 << static_cast<int>(completion->status.sc) << std::endl;
    }
    
    generator->read_completed_ = true;
}

}  // namespace benchmarking
}  // namespace nvmeof