#ifndef CONFIG_H
#define CONFIG_H

#define OK 0

#define KERNEL_TOTAL_INTERRUPTS 512
#define KERNEL_CODE_SELETOR 0x08;
#define KERNEL_DATA_SELETOR 0x10;
#define KERNEL_MAX_PATH_SIZE 256 // 256 bytes

// 100 MB at the moment
#define KERNEL_MAX_HEAP_MEMORY (1024 * 1024) * 100
#define KERNEL_HEAP_BLOCK_SIZE 4096

#define KERNEL_DEFAULT_DISK_SECTOR_SIZE 512

#endif
