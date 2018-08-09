#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>

#include "fat32.h"
#include "common.h"

void fat32_init(fat_t *p_fat, const char *device_name)
{
    setlocale(LC_CTYPE, "");

    p_fat->file_descriptor = open(device_name, O_RDONLY|O_NONBLOCK);
    if (p_fat->file_descriptor == -1)
    {
        perror("Error opening device");
        exit(EXIT_FAILURE);
    }

    p_fat->bs = malloc(sizeof(fat_BS_t));
    if(read(p_fat->file_descriptor, p_fat->bs, 90) == -1)
    {
        perror("Error reading device");
		exit(EXIT_FAILURE);
    }
    int table_size = (p_fat->bs->table_size_16 != 0) ? p_fat->bs->table_size_16 : ((fat_extBS_32_t*)p_fat->bs->extended_section)->table_size_32;
    
    p_fat->offset_fat_region = p_fat->bs->reserved_sector_count * p_fat->bs->bytes_per_sector;
    p_fat->offset_data_region = p_fat->offset_fat_region + p_fat->bs->table_count * table_size * p_fat->bs->bytes_per_sector;
    p_fat->bytes_per_cluster = p_fat->bs->sectors_per_cluster * p_fat->bs->bytes_per_sector;
    p_fat->dir_tables_in_cluster = p_fat->bs->bytes_per_sector * p_fat->bs->sectors_per_cluster / 32;

    return;
}

void fat32_teardown(fat_t *p_fat)
{
    free(p_fat->bs);
    close(p_fat->file_descriptor);
}

uint32_t fat32_get_next_cluster(fat_t *p_fat, uint32_t cluster)
{
    uint32_t next_cluster;
    read_n_bytes(p_fat->file_descriptor, &next_cluster, p_fat->offset_fat_region+32*cluster, sizeof(uint32_t));
    return next_cluster;
}

int fat32_load_dir_table_and_return_true_if_end_of_chain(fat_direntry_t *p_dir_entry, fat_file_t *p_file)
{
    if(p_dir_entry->long_file_name.attributes == 0x0F)
    {
        char *str_insert = (char*)calloc(13*4+1, sizeof(char)), *str_new;
        wchar_t wstr_insert[14];
        wstr_insert[0] = p_dir_entry->long_file_name.first_part_filename[0];
        wstr_insert[1] = p_dir_entry->long_file_name.first_part_filename[1];
        wstr_insert[2] = p_dir_entry->long_file_name.first_part_filename[2];
        wstr_insert[3] = p_dir_entry->long_file_name.first_part_filename[3];
        wstr_insert[4] = p_dir_entry->long_file_name.first_part_filename[4];
        wstr_insert[5] = p_dir_entry->long_file_name.next_part_filename[0];
        wstr_insert[6] = p_dir_entry->long_file_name.next_part_filename[1];
        wstr_insert[7] = p_dir_entry->long_file_name.next_part_filename[2];
        wstr_insert[8] = p_dir_entry->long_file_name.next_part_filename[3];
        wstr_insert[9] = p_dir_entry->long_file_name.next_part_filename[4];
        wstr_insert[10] = p_dir_entry->long_file_name.next_part_filename[5];
        wstr_insert[11] = p_dir_entry->long_file_name.final_part_filename[0];
        wstr_insert[12] = p_dir_entry->long_file_name.final_part_filename[1];
        wstr_insert[13] = 0;
        int len_insert = wcstombs(str_insert, wstr_insert, 13*4);
        str_new = realloc(p_file->basename, strlen(p_file->basename)+len_insert+1);

        memmove(str_new+len_insert, str_new, strlen(str_new)+1);
        memmove(str_new, str_insert, len_insert);
        free(str_insert);
        p_file->basename = str_new;
    }
    else
    {
        if(p_file->basename[0] == '\0')
        {
            free(p_file->basename);
            p_file->basename = (char*) calloc(13, sizeof(char));
            char filename[9], ext[4];
            int namelen = strtrimcpy(filename, (char*)p_dir_entry->standard_8_3_format.filename, 8);
            int extlen = strtrimcpy(ext, ((char*)p_dir_entry->standard_8_3_format.filename)+8, 3);
            strcpy(p_file->basename, filename);
            if(extlen > 0)
                p_file->basename[namelen++] = '.';
            strcpy(p_file->basename + namelen, ext);
        }
        p_file->dir_ent = *p_dir_entry;
    }
    return p_dir_entry->standard_8_3_format.attributes!=0x0F;
}

inline int fat32_next_cluster_chain(fat_t *p_fat, uint32_t *p_cluster)
{
    int flag_end_of_cluster = (*p_cluster&0x0FFFFFF8) == 0x0FFFFFF8;
    *p_cluster = fat32_get_next_cluster(p_fat, *p_cluster);
    return !flag_end_of_cluster;
}

uint8_t *fat32_alloc_and_load_cluster(fat_t *p_fat, uint32_t cluster)
{
    uint32_t bytes_per_cluster= p_fat->bs->bytes_per_sector * p_fat->bs->sectors_per_cluster;
    uint8_t *buffer = (uint8_t*)malloc(bytes_per_cluster);
    read_n_bytes(p_fat->file_descriptor, buffer, p_fat->offset_data_region + (cluster-2)*p_fat->bytes_per_cluster, bytes_per_cluster);
    return buffer;
}

inline int fat_dir_entry_is_deleted(fat_direntry_t *p_dir_entry)
{
    return *(uint8_t*)p_dir_entry == 0xE5;
}

inline int fat_dir_entry_is_empty(fat_direntry_t *p_dir_entry)
{
    return p_dir_entry->standard_8_3_format.attributes == 0x00;
}

void show_boot_record_info(fat_t *fat)
{
    int bytes_per_sector = fat->bs->bytes_per_sector;
    fat_extBS_32_t *p_fat_extBS_32 = (fat_extBS_32_t *)&fat->bs->extended_section;

    printf("11-12 | Bytes per sector: %u\n", bytes_per_sector);
    printf("13    | Sectors per cluster: %u\n", fat->bs->sectors_per_cluster);
    printf("14-15 | Reserved Region size (sectors): %u\n", fat->bs->reserved_sector_count);
    printf("      | -> FAT Region starts at (bytes): %#x\n", bytes_per_sector * fat->bs->reserved_sector_count);
    printf("16    | Number of File Allocation Tables copies: %u\n", fat->bs->table_count);
    printf("17-18 | Number of directory entries: %u\n", fat->bs->root_entry_count);

    printf("19-20 | The total sectors in the logical volume: %#x\n", fat->bs->total_sectors_16 | fat->bs->total_sectors_32);
    printf("/32-35| -> The size in the logical volume (bytes): %u\n", bytes_per_sector * (fat->bs->total_sectors_16 | fat->bs->total_sectors_32));

    //extended fat32 stuff
    printf("36-39 | The size of the FAT (sectors): %u\n", p_fat_extBS_32->table_size_32);
    printf("      | -> Data Region starts at (bytes): %#x\n", bytes_per_sector * (fat->bs->reserved_sector_count + fat->bs->table_count*p_fat_extBS_32->table_size_32));
    printf("44-47 | The cluster number of the root directory: %u\n", p_fat_extBS_32->root_cluster);
    printf("48-49 | Offset of the FSInfo structure in Reserved Region (sectors): %u\n", p_fat_extBS_32->root_cluster);
    printf("50-51 | The sector number of the backup boot sector: %u -> %#x\n", p_fat_extBS_32->backup_BS_sector, bytes_per_sector*p_fat_extBS_32->backup_BS_sector);
    
    return;
}

void show_dir_entry_info(fat_direntry_t *p_dir_entry)
{
    printf("0-10  | 8.3 file name: %8s\n", p_dir_entry->standard_8_3_format.filename);
    printf("11    | Attributes of the file: %#x\n", p_dir_entry->standard_8_3_format.attributes);
    printf("14-15 | Creation time: %02u:%02u:%02u\n", p_dir_entry->standard_8_3_format.created_time.hour,
        p_dir_entry->standard_8_3_format.created_time.minute,
        2 * p_dir_entry->standard_8_3_format.created_time.second);
    printf("16-17 | Creation time: %u/%02u/%02u\n", 1980 + p_dir_entry->standard_8_3_format.created_date.year,
        p_dir_entry->standard_8_3_format.created_date.month,
        2 * p_dir_entry->standard_8_3_format.created_date.day);
    printf("26-27 | The entry's first cluster number: %u\n", ((uint32_t)p_dir_entry->standard_8_3_format.high_cluster)<<16 | p_dir_entry->standard_8_3_format.low_cluster);
    printf("28-31 | The size of the file (bytes): %u\n", p_dir_entry->standard_8_3_format.file_size);
    return;
}

void fat_file_init(fat_file_t *p_file)
{
    p_file->basename = (char*) calloc(1, sizeof(char));
}

inline void fat_file_destruct(fat_file_t *p_file)
{
    // free(p_file->dirname);
    free(p_file->basename);
}

inline int fat_file_is_directory(fat_file_t *p_file)
{
    return p_file->dir_ent.standard_8_3_format.attributes == 0x10;
}

inline int fat_file_is_dot_entry(fat_file_t *p_file)
{
    return *((uint8_t*)&p_file->dir_ent) == 0x2E;
}

inline int fat_file_is_file(fat_file_t *p_file)
{
    return p_file->dir_ent.standard_8_3_format.attributes!=0 && p_file->dir_ent.standard_8_3_format.attributes != 0x10;
}

uint32_t fat_file_get_cluster(fat_file_t *p_file)
{
    return (uint32_t)p_file->dir_ent.standard_8_3_format.high_cluster << 16 | p_file->dir_ent.standard_8_3_format.low_cluster;
}

int strtrimcpy(char *dest, const char *src, int n){
	int len=0;
	while(src[len] != ' ' && len<n && (dest[len] = src[len++] ));
	dest[len]=0;
	return len;
}