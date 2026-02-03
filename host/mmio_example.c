/**
 * Simple host MMIO example for Alveo V80 (AVED/AMI).
 * Reads and writes 32-bit FPGA registers via PCIe BAR.
 *
 * Build on the node (see Makefile). Requires AMI API (headers + lib)
 * from /opt/amd or your AVED RPM install. Run with sudo for BAR access.
 *
 * Usage:
 *   sudo ./mmio_example <BDF> [bar] [base_offset]
 * Example:
 *   sudo ./mmio_example 21:00.0
 *   sudo ./mmio_example 21 0 0x1000
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* AMI API - adjust include path to match your install (/opt/amd or RPM) */
#include <ami.h>

#define DEFAULT_BAR   0
#define DEFAULT_OFFSET 0x0

static void usage(const char *prog)
{
    fprintf(stderr, "Usage: %s <BDF> [bar] [base_offset]\n", prog);
    fprintf(stderr, "  BDF         e.g. 21:00.0 or 21\n");
    fprintf(stderr, "  bar         BAR index (default %u)\n", DEFAULT_BAR);
    fprintf(stderr, "  base_offset Hex offset in BAR (default 0x%x)\n", DEFAULT_OFFSET);
    fprintf(stderr, "Requires root for BAR access.\n");
}

int main(int argc, char **argv)
{
    ami_device *dev = NULL;
    const char *bdf;
    uint8_t bar_idx = DEFAULT_BAR;
    uint64_t base_offset = DEFAULT_OFFSET;
    uint32_t val;
    int ret;

    if (argc < 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    bdf = argv[1];
    if (argc >= 3)
        bar_idx = (uint8_t)strtoul(argv[2], NULL, 0);
    if (argc >= 4)
        base_offset = strtoull(argv[3], NULL, 16);

    ret = ami_dev_find(bdf, &dev);
    if (ret != AMI_STATUS_OK) {
        fprintf(stderr, "ami_dev_find failed: %s\n", ami_get_last_error());
        return EXIT_FAILURE;
    }

    ret = ami_dev_request_access(dev);
    if (ret != AMI_STATUS_OK) {
        fprintf(stderr, "ami_dev_request_access failed (need root?): %s\n", ami_get_last_error());
        ami_dev_delete(&dev);
        return EXIT_FAILURE;
    }

    /* Read one register at base_offset */
    ret = ami_mem_bar_read(dev, bar_idx, base_offset, &val);
    if (ret != AMI_STATUS_OK) {
        fprintf(stderr, "ami_mem_bar_read failed: %s\n", ami_get_last_error());
        ami_dev_delete(&dev);
        return EXIT_FAILURE;
    }
    printf("Read  BAR%u @ 0x%llx = 0x%08x\n", bar_idx, (unsigned long long)base_offset, val);

    /* Write a test value, then read back (offset + 4 = next 32-bit register if your IP has it) */
    val = 0xdeadbeef;
    ret = ami_mem_bar_write(dev, bar_idx, base_offset, val);
    if (ret != AMI_STATUS_OK) {
        fprintf(stderr, "ami_mem_bar_write failed: %s\n", ami_get_last_error());
        ami_dev_delete(&dev);
        return EXIT_FAILURE;
    }
    printf("Wrote BAR%u @ 0x%llx = 0x%08x\n", bar_idx, (unsigned long long)base_offset, val);

    ret = ami_mem_bar_read(dev, bar_idx, base_offset, &val);
    if (ret != AMI_STATUS_OK) {
        fprintf(stderr, "ami_mem_bar_read (readback) failed: %s\n", ami_get_last_error());
        ami_dev_delete(&dev);
        return EXIT_FAILURE;
    }
    printf("Read  BAR%u @ 0x%llx = 0x%08x (readback)\n", bar_idx, (unsigned long long)base_offset, val);

    ami_dev_delete(&dev);
    printf("Done.\n");
    return 0;
}
