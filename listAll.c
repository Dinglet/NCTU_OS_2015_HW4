#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "common.h"
#include "fat32.h"

// typedef struct file_node_s file_node_t;

// void file_node_destruct(file_node_t *p_node);
void fat_listAll(fat_t *p_fat, fat_file_t *p_file);
// void file_node_listAll(file_node_t *root);

int main(int argc, char **argv) 
{
    int device_fd;
    const char *device_name;
    
    if(argc < 2)
    {
        perror("Usage: ./listAll IMAGE");
        exit(EXIT_FAILURE);
    }
    device_name = argv[1];
    
    fat_t fat;
    fat32_init(&fat, device_name);

    fat_file_t file;
    file.dirname = (char *) calloc(1, sizeof(char));
    file.basename = (char *) calloc(2, sizeof(char));
    file.basename[0] = '.';
    file.dir_ent.standard_8_3_format.filename[0] = 0x2E;
    file.dir_ent.standard_8_3_format.attributes = 0x10;
    file.dir_ent.standard_8_3_format.low_cluster = 2;
    
    fat_listAll(&fat, &file);

    free(file.dirname);
    fat_file_destruct(&file);
    fat32_teardown(&fat);

    return 0;
}

void fat_listAll(fat_t *p_fat, fat_file_t *p_file)
{
    // Return if it is a data file
    if(fat_file_is_file(p_file))
        return;
    // Store the cluster content
    // meanwhile for each file or directory, call this function again
    uint32_t cluster = fat_file_get_cluster(p_file);
    fat_file_t file_current;
    fat_direntry_t *buffer;
    int i_dir_table;

    int len_dirname = strlen(p_file->dirname), len_basename = strlen(p_file->basename);
    int len_cat = len_dirname + len_basename + 2;
    char *dirname_current = (char*)malloc(len_cat * sizeof(char));
    memcpy(dirname_current, p_file->dirname, len_dirname);
    memcpy(dirname_current+len_dirname, p_file->basename, len_basename);
    dirname_current[len_dirname + len_basename] = '/';
    dirname_current[len_cat] = '\0';
    
    fat_file_init(&file_current);
    do
    {
        buffer = fat32_alloc_and_load_cluster(p_fat, cluster);
        for(i_dir_table=0; i_dir_table<p_fat->dir_tables_in_cluster; ++i_dir_table)
        {
            if(fat_dir_entry_is_deleted(buffer+i_dir_table))
                continue;
            if(fat_dir_entry_is_empty(buffer+i_dir_table))
                continue;
            if(fat32_load_dir_table_and_return_true_if_end_of_chain(buffer+i_dir_table, &file_current))
            {
                if(!fat_file_is_dot_entry(&file_current))
                {
                    file_current.dirname = dirname_current;
                    // Print current filename
                    printf("%s%s\n", file_current.dirname,file_current.basename);
                    fat_listAll(p_fat, &file_current);
                }
                fat_file_destruct(&file_current);
                fat_file_init(&file_current);
            }
        }
        free(buffer);
    } while(fat32_next_cluster_chain(p_fat, &cluster));
    fat_file_destruct(&file_current);
    free(dirname_current);
}
