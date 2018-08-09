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

    // this will be cast to it's specific type once the 
    // driver actually knows what type of FAT this is.
	uint8_t		extended_section[54];   // 36   54
}__attribute__((packed)) fat_BS_t;      // 90 bytes

typedef struct fat_extBS_32
{
    //extended fat32 stuff
    uint32_t    table_size_32;          // 36   4   Sectors per FAT. The size of the FAT in sectors. 
    uint16_t    extended_flags;         // 40   2
    uint16_t    fat_version;            // 42   2
    uint32_t    root_cluster;           // 44   4   The cluster number of the root directory.
    uint16_t    fat_info;               // 48   2   The sector number of the FSInfo structure.
    uint16_t    backup_BS_sector;       // 50   2
    uint8_t     reserved_0[12];         // 52   12
    uint8_t     drive_number;           // 64   1
    uint8_t     reserved_1;             // 65   1
    uint8_t     boot_signature;         // 66   1
    uint32_t    volume_id;              // 67   4
    uint8_t     volume_label[11];       // 71   11
    uint8_t     fat_type_label[8];      // 82   8
}__attribute__((packed)) fat_extBS_32_t;// 54 bytes

typedef struct fat_FSInfoS
{
    uint8_t     lead_signature[4];      // 0    4
    uint8_t     reserved_1[480];        // 4    480
    uint8_t     structure_signature[4];
    uint32_t    free_count;
    uint32_t    next_free;
    uint8_t     reserved_2[12];
    uint8_t     tail_signature[4];
}__attribute__((packed)) fat_FSInfoS_t;

typedef union fat_direntry
{
    struct __attribute__((packed))
    {
        struct time
        {
            uint16_t second:5;
            uint16_t minute:6;
            uint16_t hour:5;
        };
        struct date
        {
            uint16_t day:5;
            uint16_t month:4;
            uint16_t year:7;
        };
        uint8_t filename[11];
        uint8_t attributes;
        uint8_t reserved_winNT;
        uint8_t created_time_tenth_second;
        struct time created_time;
        struct date created_date;
        uint16_t last_accessed_date;
        uint16_t high_cluster;
        struct time last_modified_time;
        struct date clast_modified_date;
        uint16_t low_cluster;
        uint32_t file_size;
    } standard_8_3_format;
    struct __attribute__((packed))
    {
        union
        {
            struct
            {
                uint8_t number:5;
                uint8_t reserved_0:1;
                uint8_t first_physical_LFN_entry:1;
                uint8_t reserved_1:1;
            } bits;
            uint8_t val;
        } sequence_number;
        uint16_t first_part_filename[5];
        uint8_t attributes; // always 0x0f if it is LFN
        uint8_t type;
        uint8_t checksum;
        uint16_t next_part_filename[6];
        uint8_t reserved_0[2]; // always zero
        uint16_t final_part_filename[2];
    } long_file_name;
}fat_direntry_t;

typedef struct fat_s {
    int file_descriptor;
    fat_BS_t *bs;
    uint32_t offset_fat_region, offset_data_region;
    uint32_t bytes_per_cluster;
    uint32_t dir_tables_in_cluster;
    // fat_fsinfo_t *info;
    // int fs_type;
    int data_sect;
    // int n_clusters;
} fat_t;

typedef struct fat_file {
    char            *dirname, *basename;
    fat_direntry_t  dir_ent;
    // int             offset;
    // int             beg_marker;
    // int             eof_marker;
} fat_file_t;

void fat32_init(fat_t *fat, const char *device_name);
void fat32_teardown(fat_t *fat);
uint32_t fat32_get_next_cluster(fat_t *p_fat, uint32_t cluster);
int fat32_load_dir_table_and_return_true_if_end_of_chain(fat_direntry_t *p_dir_entry, fat_file_t *p_file);
int fat32_next_cluster_chain(fat_t *p_fat, uint32_t *p_cluster);
uint8_t *fat32_alloc_and_load_cluster(fat_t *p_fat, uint32_t cluster);

int fat_dir_entry_is_deleted(fat_direntry_t *p_dir_entry);
int fat_dir_entry_is_empty(fat_direntry_t *p_dir_entry);

void show_boot_record_info(fat_t *fat);
void show_dir_entry_info(fat_direntry_t *p_dir_entry);

void fat_file_init(fat_file_t *p_file);
void fat_file_destruct(fat_file_t *p_file);
int fat_file_is_directory(fat_file_t *p_file);
int fat_file_is_dot_entry(fat_file_t *p_file);
int fat_file_is_file(fat_file_t *p_file);
uint32_t fat_file_get_cluster(fat_file_t *p_file);

int strtrimcpy(char *dest, const char *src, int n);
