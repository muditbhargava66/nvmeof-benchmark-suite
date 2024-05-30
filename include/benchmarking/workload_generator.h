#pragma once

#include <cstdint>
#include <string>
#include <spdk/nvme.h>

namespace nvmeof {
namespace benchmarking {

struct WorkloadProfile {
    uint64_t total_size;
    uint32_t block_size;
    uint32_t num_blocks;
    uint32_t interval_us;
};

class WorkloadGenerator {
public:
    WorkloadGenerator(const struct spdk_nvme_ctrlr *ctrlr, const struct spdk_nvme_qpair *qpair,
                      const WorkloadProfile& profile);

    void Generate();

private:
    void WriteBlock(uint64_t offset, uint32_t size);
    static void WriteCompletionCallback(void *arg, const struct spdk_nvme_cpl *completion);

    const struct spdk_nvme_ctrlr *ctrlr_;
    const struct spdk_nvme_qpair *qpair_;
    WorkloadProfile profile_;
    bool write_completed_;
};

}  // namespace benchmarking
}  // namespace nvmeof