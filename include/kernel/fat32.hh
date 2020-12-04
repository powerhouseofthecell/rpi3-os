#ifndef FAT32_HH
#define FAT32_HH

#include "common/types.hh"

/*
    For fun reading: https://tc.gts3.org/cs3210/2020/spring/r/fat-structs.pdf
*/

#define MBR_SIGNATURE 0xaa55
#define FAT32_PARTITION_TYPE1 0xb
#define FAT32_PARTITION_TYPE2 0xc
#define EBPB_SIGNATURE1 0x28
#define EBPB_SIGNATURE2 0x29

// the partition entry for the master boot record
typedef struct {
    uint8_t boot_indicator;
    uint8_t starting_head;      // typically ignored
    uint16_t starting_cylinder; // typically ignored
    uint8_t partition_type;
    uint8_t ending_head;        // typically ignored
    uint16_t ending_cylinder;   // typically ignored
    uint32_t relative_sector;
    uint32_t total_sectors;
} __attribute__((packed)) fat32_partition_entry;

// the master boot record
typedef struct {
    uint8_t mbr_bootstrap[436];
    uint8_t disk_id[10];

    fat32_partition_entry partition_table[4];

    uint16_t signature;
} __attribute__((packed)) fat32_mbr;

// the extended bios parameter block
typedef struct {
    uint8_t instr[3];           // EB, XX, 90 ==> disassemble to JMP SHORT XX NOP
    uint64_t oem_id;
    uint16_t bytes_per_sector;  // NOTE: little endian format
    uint8_t sectors_per_cluster;
    uint16_t num_reserved_sectors;
    uint8_t num_fats;
    uint16_t max_dir_entries;
    uint16_t total_logical_sectors_small;
    uint8_t fat_id;
    uint16_t sectors_per_fat_small;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t begin_partition_lba;
    uint32_t total_logical_sectors_large;
    uint32_t sectors_per_fat_large;
    uint16_t flags;
    uint16_t fat_version;
    uint32_t root_cluster_no;
    uint16_t fsinfo_sector_no;
    uint16_t backup_boot_sector_no;
    uint32_t reserved[3];
    uint8_t drive_no;
    uint8_t windows_nt_flags;
    uint8_t signature;
    uint32_t volume_id;
    char volume_label[11];  // padded with spaces
    char sys_identifier[8];
    uint8_t boot_code[420];
    uint16_t bootable_signature;
} __attribute__((packed)) fat32_ebpb;

#define DIRENT_READ_ONLY    0x01
#define DIRENT_HIDDEN       0x02
#define DIRENT_SYSTEM       0x04
#define DIRENT_VOLUME_ID    0x08
#define DIRENT_DIRECTORY    0x10
#define DIRENT_ARCHIVE      0x20
#define DIRENT_LFN          0x0F

typedef struct {
    char filename[8];
    char file_ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_accessed;
    uint16_t first_cluster_no_high;
    uint16_t last_modified_time;
    uint16_t last_modified_date;
    uint16_t first_cluster_no_low;
    uint32_t file_size;
} __attribute__((packed)) fat32_dir_entry;

typedef struct {
    uint8_t seq_no;
    char first_name_chars[10];
    uint8_t attributes;         // must be 0xF
    uint8_t type;
    uint8_t checksum;
    char second_name_chars[12];
    uint16_t zero;              // must be 0x00
    char third_name_chars[4];
} __attribute__((packed)) fat32_long_filename;

// a class for instantiating disk objects and reading/writing to them
class sditer {
    // all operations except constructor on this object should fail iff false
    bool valid = false;

    // the master boot record object
    fat32_mbr mbr;

    // the extended boot partition block object
    fat32_ebpb ebpb;

    // the base sector of the FAT
    uint32_t fat_base_sector = 0x00;
    // which sector off the base sector is currently in the fat buffer, -1 if not loaded
    int fat_off_sector = -1;

    // the base sector of the data section
    uint32_t data_base_sector = 0x00;
    // which sector off the base sector is currently in the data buffer, -1 if not loaded
    int data_off_sector = -1;

    // which cluster is currently in data_buf
    int current_cluster_no = -1;

    // a buffer for holding a fat sector in memory
    uint32_t fat_buf[128];

    // a buffer for holding a data sector in memory
    uint32_t data_buf[128];

public:
    sditer();
};

// a global version to read the attached sd card
extern sditer base_sd_device;

#endif