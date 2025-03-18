/**
 * @file nvme.h
 * @brief Mock SPDK NVMe header for macOS development
 * 
 * This is a simplified mock implementation of the SPDK NVMe API for development
 * purposes on macOS. It provides the minimal set of structures and function
 * declarations required for the NVMe-oF Benchmarking Suite to compile.
 * 
 * For actual benchmarking, use the real SPDK in a Linux environment or Docker.
 */

 #pragma once

 #include <stdint.h>
 #include <stdbool.h>
 #include <stddef.h>  /* For size_t */
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /**
  * @brief Mock NVMe controller structure
  */
 struct spdk_nvme_ctrlr {
     uint32_t id;
     void* userdata;
     /* Minimal set of fields needed */
 };
 
 /**
  * @brief Mock NVMe queue pair structure
  */
 struct spdk_nvme_qpair {
     uint32_t id;
     void* userdata;
     /* Minimal set of fields needed */
 };
 
 /**
  * @brief Mock NVMe namespace structure
  */
 struct spdk_nvme_ns {
     uint32_t id;
     uint32_t sector_size;
     /* Minimal set of fields needed */
 };
 
 /**
  * @brief Mock NVMe completion structure
  */
 struct spdk_nvme_cpl {
     struct {
         uint16_t status_code_type : 3;
         uint16_t status_code : 8;
         uint16_t p : 1;
         uint16_t sc : 4;  /* Status Code */
     } status;
     /* Minimal set of fields needed */
 };
 
 /**
  * @brief Get a namespace from the controller
  * 
  * @param ctrlr Controller
  * @param ns_id Namespace ID
  * @return Pointer to namespace structure
  */
 struct spdk_nvme_ns* spdk_nvme_ctrlr_get_ns(const struct spdk_nvme_ctrlr* ctrlr, uint32_t ns_id);
 
 /**
  * @brief Get sector size of a namespace
  * 
  * @param ns Namespace
  * @return Sector size in bytes
  */
 uint32_t spdk_nvme_ns_get_sector_size(const struct spdk_nvme_ns* ns);
 
 /**
  * @brief Check if completion has an error
  * 
  * @param cpl Completion structure
  * @return true if there's an error, false otherwise
  */
 bool spdk_nvme_cpl_is_error(const struct spdk_nvme_cpl* cpl);
 
 /**
  * @brief Process completions on a queue pair
  * 
  * @param qpair Queue pair
  * @param max_completions Maximum number of completions to process
  * @return Number of completions processed
  */
 int32_t spdk_nvme_qpair_process_completions(struct spdk_nvme_qpair* qpair, uint32_t max_completions);
 
 /**
  * @brief Allocate DMA-capable memory
  * 
  * @param size Size in bytes
  * @param alignment Alignment
  * @param phys_addr Physical address (output)
  * @return Pointer to allocated memory
  */
 void* spdk_dma_malloc(size_t size, size_t alignment, uint64_t* phys_addr);
 
 /**
  * @brief Free DMA-capable memory
  * 
  * @param buf Pointer to memory allocated with spdk_dma_malloc
  */
 void spdk_dma_free(void* buf);
 
 /**
  * @brief Submit a read command to a namespace
  * 
  * @param ns Namespace
  * @param qpair Queue pair
  * @param buffer Data buffer
  * @param lba Starting LBA
  * @param lba_count Number of blocks to read
  * @param cb_fn Completion callback function
  * @param cb_arg Argument for callback function
  * @param io_flags I/O flags
  * @return 0 on success, negative errno on failure
  */
 int spdk_nvme_ns_cmd_read(struct spdk_nvme_ns* ns, struct spdk_nvme_qpair* qpair,
                           void* buffer, uint64_t lba, uint32_t lba_count,
                           void (*cb_fn)(void* cb_arg, const struct spdk_nvme_cpl* cpl),
                           void* cb_arg, uint32_t io_flags);
 
 /**
  * @brief Submit a write command to a namespace
  * 
  * @param ns Namespace
  * @param qpair Queue pair
  * @param buffer Data buffer
  * @param lba Starting LBA
  * @param lba_count Number of blocks to write
  * @param cb_fn Completion callback function
  * @param cb_arg Argument for callback function
  * @param io_flags I/O flags
  * @return 0 on success, negative errno on failure
  */
 int spdk_nvme_ns_cmd_write(struct spdk_nvme_ns* ns, struct spdk_nvme_qpair* qpair,
                            void* buffer, uint64_t lba, uint32_t lba_count,
                            void (*cb_fn)(void* cb_arg, const struct spdk_nvme_cpl* cpl),
                            void* cb_arg, uint32_t io_flags);
 
 #ifdef __cplusplus
 }
 #endif
