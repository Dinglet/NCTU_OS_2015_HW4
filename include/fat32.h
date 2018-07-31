/*
 * @file: fat32.h
 * 
 * @author: Kevin Allison
 * @date: 11 Apr. 2013
 * @modifier: Shien-Ting Huang
 * @date: 4 Jul. 2016
 *
 * Contains structures and function definitions
 * for the FAT32 filesystem
 */
#pragma once

#include <stdint.h>
//#include "fat_types.h"
typedef struct fat_BS
{
    uint8_t     bootjmp[3];             // 0    3   
    uint8_t     oem_name[8];            // 3    8
    uint16_t    bytes_per_sector;       // 11   2   The number of Bytes per sector
    uint8_t     sectors_per_cluster;    // 13   1   Number of sectors per cluster.
    uint16_t    reserved_sector_count;  // 14   2
    uint8_t     table_count;            // 16   1
    uint16_t    root_entry_count;       // 17   2
    uint16_t    total_sectors_16;       // 19   2
    uint8_t     media_type;             // 21   1
    uint16_t    table_size_16;          // 22   2
    uint16_t    sectors_per_track;      // 24   2
    uint16_t    head_side_count;        // 26   2
    uint32_t    hidden_sector_count;    // 28   4
    uint32_t    total_sectors_32;       // 32   4   Large amount of sector on media. This field is set if there are more than 65535 sectors in the volume. 

    //extended fat32 stuff
    uint32_t    table_size_32;          // 36   4   Sectors per FAT. The size of the FAT in sectors. 
    uint16_t    extended_flags;         // 40   2
    uint16_t    fat_version;            // 42   2
    uint32_t    root_cluster;           // 44   4   The cluster number of the root directory. Often this field is set to 2.
    uint16_t    fat_info;               // 48   2   The sector number of the FSInfo structure.
    uint16_t    backup_BS_sector;       // 50   2
    uint8_t     reserved_0[12];         // 52   12
    uint8_t     drive_number;           // 64   1
    uint8_t     reserved_1;             // 65   1
    uint8_t     boot_signature;         // 66   1
    uint32_t    volume_id;              // 67   4
    uint8_t     volume_label[11];       // 71   11
    uint8_t     fat_type_label[8];      // 82   8
    uint8_t     boot_code[420];         // 90   420
    uint8_t     partition_signature[2]; // 510  2
}__attribute__((packed)) fat_BS_t;

typedef struct fat_FSInfoS
{
    uint8_t     lead_signature[4];      // 0    4
    uint8_t     reserved_1[480];        // 4    480
    uint8_t     structure_signature[4];
    uint32_t    free_count;
    uint32_t    nexxt_free;
    uint8_t     reserved_2[12];
    uint8_t     tail_signature[4];
}__attribute__((packed)) fat_FSInfoS_t;

typedef union dir_entry
{
    struct __attribute__((packed))
    {
        struct time
        {
            uint16_t hour:5;
            uint16_t minute:6;
            uint16_t second:5;
        };
        struct date
        {
            uint16_t year:7;
            uint16_t month:4;
            uint16_t day:5;
        };
        uint8_t filename[11];
        uint8_t attributes;
        uint8_t reserved_winNT;
        uint8_t created_time_tenth_second;
        union
        {
            struct time bit;
            uint16_t reg;
        } created_time;
        union
        {
            struct date bit;
            uint16_t reg;
        } created_date;
        uint16_t last_accessed_date;
        uint16_t high_cluster;
        union
        {
            struct time bit;
            uint16_t reg;
        } last_modified_time;
        union
        {
            struct date bit;
            uint16_t reg;
        } clast_modified_date;
        uint16_t low_cluster;
        uint32_t file_size;
    } standard_8_3_format;
    struct __attribute__((packed))
    {
        union
        {
            struct
            {
                uint8_t reserved_0:1;
                uint8_t first_physical_LFN_entry:1;
                uint8_t reserved_1:1;
                uint8_t sequence_number:5;
            } bit;
            uint8_t reg;
        } sequence_number;
        uint16_t first_part_filename[5];
        uint8_t attributes; // always 0x0f if it is LFN
        uint8_t type;
        uint8_t checksum;
        uint16_t next_part_filename[6];
        uint8_t reserved_0[2]; // always zero
        uint16_t final_part_filename[2];
    } long_file_name;
}dir_entry_t;
