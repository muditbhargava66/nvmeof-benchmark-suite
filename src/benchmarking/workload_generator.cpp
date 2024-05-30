#include "benchmarking/workload_generator.h"
#include <iostream>
#include <random>
#include <chrono>
#include <thread>

namespace nvmeof {
namespace benchmarking {

WorkloadGenerator::WorkloadGenerator(const WorkloadProfile& profile)
    : profile_(profile) {}

void WorkloadGenerator::Generate() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, profile_.num_blocks - 1);

    auto start_time = std::chrono::high_resolution_clock::now();
    int64_t total_bytes_written = 0;

    while (total_bytes_written < profile_.total_size) {
        uint64_t block_offset = dist(gen) * profile_.block_size;
        uint32_t block_size = std::min(profile_.block_size, profile_.total_size - total_bytes_written);

        // Perform write operation
        WriteBlock(block_offset, block_size);

        total_bytes_written += block_size;

        // Sleep for the specified interval
        std::this_thread::sleep_for(std::chrono::microseconds(profile_.interval_us));
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "Workload generation completed." << std::endl;
    std::cout << "Total bytes written: " << total_bytes_written << std::endl;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;
}

/*
Exampole Usuage:

static void WriteCompletionCallback(void *arg, const struct spdk_nvme_cpl *completion) {
    if (spdk_nvme_cpl_is_error(completion)) {
        std::cerr << "Error: Write operation failed" << std::endl;
    }
    write_completed_ = true;
}
*/

void WorkloadGenerator::WriteBlock(uint64_t offset, uint32_t size) {
    struct spdk_nvme_ns *ns = spdk_nvme_ctrlr_get_ns(ctrlr_, 1);
    if (ns == nullptr) {
        std::cerr << "Error: Namespace not found" << std::endl;
        return;
    }

    // Allocate a buffer for writing data
    void *buffer = spdk_dma_malloc(size, 0, nullptr);
    if (buffer == nullptr) {
        std::cerr << "Error: Memory allocation failed" << std::endl;
        return;
    }

    // Fill the buffer with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 255);
    uint8_t *data = static_cast<uint8_t *>(buffer);
    for (uint32_t i = 0; i < size; ++i) {
        data[i] = dist(gen);
    }

    // Submit the write operation
    uint64_t lba = offset / spdk_nvme_ns_get_sector_size(ns);
    uint32_t lba_count = size / spdk_nvme_ns_get_sector_size(ns);

    spdk_nvme_qpair_io_write(qpair_, ns, buffer, lba, lba_count, WriteCompletionCallback, nullptr, 0);

    // Poll for completions
    while (!write_completed_) {
        spdk_nvme_qpair_process_completions(qpair_, 0);
    }

    // Reset the completion flag
    write_completed_ = false;

    // Free the buffer
    spdk_dma_free(buffer);
}

}  // namespace benchmarking
}  // namespace nvmeof