#include "kernel/fat32.hh"
#include "kernel/sd.hh"

#include "common/stdlib.hh"

    // // get the FAT
    // sd_readblock(0x2020, buf, 1);
    // uint32_t* fat = (uint32_t*) buf;
    // for (int i = 0; i < 8; ++i) {
    //     printf("fat[%i]: 0x%x\n", i, fat[i]);
    // }

    // get a particular entry
    //sd_readblock()
sditer::sditer() {
    // initialize the sd card
    if (sd_init() != SD_OK) {
        this->valid = false;
        printf("SD Card failure\n");
        return;
    }
    printf("SD Card initialized\n");

    // get the master boot record and verify signature
    sd_readblock(0, (unsigned char*) &this->mbr, 1);
    if (this->mbr.signature != MBR_SIGNATURE) {
        this->valid = false;
        printf("MBR Signature invalid\n");
        return;
    }
    printf("MBR Signature verified\n");

    // also verify that the 0th partition is a fat32 partition
    if (this->mbr.partition_table[0].partition_type != FAT32_PARTITION_TYPE1 &&
        this->mbr.partition_table[0].partition_type != FAT32_PARTITION_TYPE2
    ) {
        this->valid = false;
        printf("Partition Table Entry 0 has invalid type: 0x%x\n", this->mbr.partition_table[0].partition_type);
        return;
    }

    // get the extended boot partition block and verify signature
    sd_readblock(this->mbr.partition_table[0].relative_sector, (unsigned char*) &this->ebpb, 1);
    if (this->ebpb.signature != EBPB_SIGNATURE1 && this->ebpb.signature != EBPB_SIGNATURE2) {
        this->valid = false;
        printf("EBPB Signature invalid\n");
        return;
    }
    printf("EPBP Signature Verified\n");
}