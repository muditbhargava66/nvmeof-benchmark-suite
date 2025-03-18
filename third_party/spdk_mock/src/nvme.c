/**
 * @file nvme.c
 * @brief Mock SPDK NVMe implementation for macOS development
 * 
 * This provides a simplified mock implementation of the SPDK NVMe API functions
 * for development purposes on macOS. The functions simulate success but don't
 * perform any actual NVMe operations.
 */

//  #include <spdk/nvme.h>
 #include "../include/nvme.h"
 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>
  
 // Global variables for mock implementation
 static struct spdk_nvme_ns g_mock_ns = {
     .id = 1,
     .sector_size = 4096  // Standard 4K sector size
 };
  
 struct spdk_nvme_ns* spdk_nvme_ctrlr_get_ns(const struct spdk_nvme_ctrlr* ctrlr, uint32_t ns_id) {
     // Mark parameters as used to suppress warnings
     (void)ctrlr;
     (void)ns_id;
     
     // Always return our mock namespace
     return &g_mock_ns;
 }
  
 uint32_t spdk_nvme_ns_get_sector_size(const struct spdk_nvme_ns* ns) {
     if (ns == NULL) {
         return 0;
     }
     return ns->sector_size;
 }
  
 bool spdk_nvme_cpl_is_error(const struct spdk_nvme_cpl* cpl) {
     // Mark parameter as used to suppress warnings
     (void)cpl;
     
     // For mock implementation, we'll say there's no error
     return false;
 }
  
 int32_t spdk_nvme_qpair_process_completions(struct spdk_nvme_qpair* qpair, uint32_t max_completions) {
     // Mark parameters as used to suppress warnings
     (void)qpair;
     (void)max_completions;
     
     // Mock processing 1 completion
     return 1;
 }
  
 void* spdk_dma_malloc(size_t size, size_t alignment, uint64_t* phys_addr) {
     // Mark alignment parameter as used to suppress warnings
     (void)alignment;
     
     // Just use regular malloc for the mock
     void* ptr = malloc(size);
     if (phys_addr) {
         // In a real implementation, this would be a physical address
         *phys_addr = (uint64_t)ptr;
     }
     return ptr;
 }
  
 void spdk_dma_free(void* buf) {
     // Just use regular free for the mock
     free(buf);
 }
  
 int spdk_nvme_ns_cmd_read(struct spdk_nvme_ns* ns, struct spdk_nvme_qpair* qpair,
                          void* buffer, uint64_t lba, uint32_t lba_count,
                          void (*cb_fn)(void* cb_arg, const struct spdk_nvme_cpl* cpl),
                          void* cb_arg, uint32_t io_flags) {
     // Mark unused parameters to suppress warnings
     (void)qpair;
     (void)io_flags;
     
     // Simulate successful read by filling buffer with some pattern
     if (buffer && lba_count > 0) {
         // Fill with a simple pattern based on LBA
         memset(buffer, (int)(lba & 0xFF), lba_count * (ns ? ns->sector_size : 4096));
     }
     
     // Call the completion callback if provided
     if (cb_fn) {
         struct spdk_nvme_cpl cpl = {0};  // Zero-initialized (success)
         cb_fn(cb_arg, &cpl);
     }
     
     return 0;  // Success
 }
  
 int spdk_nvme_ns_cmd_write(struct spdk_nvme_ns* ns, struct spdk_nvme_qpair* qpair,
                           void* buffer, uint64_t lba, uint32_t lba_count,
                           void (*cb_fn)(void* cb_arg, const struct spdk_nvme_cpl* cpl),
                           void* cb_arg, uint32_t io_flags) {
     // Mark unused parameters to suppress warnings
     (void)ns;
     (void)qpair;
     (void)buffer;
     (void)lba;
     (void)lba_count;
     (void)io_flags;
     
     // Simulate successful write (no actual operation)
     
     // Call the completion callback if provided
     if (cb_fn) {
         struct spdk_nvme_cpl cpl = {0};  // Zero-initialized (success)
         cb_fn(cb_arg, &cpl);
     }
     
     return 0;  // Success
 }
