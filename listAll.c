#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include "common.h"
#include "fat32.h"

// typedef struct file_node_s file_node_t;

// void file_node_destruct(file_node_t *p_node);
void fat_listAll(const fat_t *p_fat, fat_file_t *p_file);
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
    
    /** OPEN the looper or images using System API */
    // if ((device_fd = open(device_name, O_RDONLY|O_NONBLOCK)) == -1)
    // {
    //     perror("Error opening image");
    //     exit(EXIT_FAILURE);
    // }
    

    fat_t fat;
    // fat.bs = malloc(sizeof(fat_BS_t));
    // if(read(device_fd, fat.bs, 90) == -1)
    // {
    //     close(device_fd);
    //     perror("Error reading image");
	// 	exit(EXIT_FAILURE);
    // }

    // show_boot_record_info(&fat);
    // free(fat.bs);

    // fat_direntry_t *buffer = (fat_direntry_t *)malloc(sizeof(fat_direntry_t));
    // lseek(device_fd, 0x103800, SEEK_SET);
    // if(read(device_fd, buffer, 32) == -1)
    // {
    //     close(device_fd);
    //     perror("Error reading image");
	// 	exit(EXIT_FAILURE);
    // }
    // show_dir_entry_info(buffer);
    // read(device_fd, buffer, 32);
    // show_dir_entry_info(buffer);
    // read(device_fd, buffer, 32);
    // show_dir_entry_info(buffer);
    // free(buffer);

    // close(device_fd);


    fat32_init(&fat, device_name);

    fat_file_t file;
    file.dirname = (char *) calloc(1, sizeof(char));
    file.basename = (char *) calloc(2, sizeof(char));
    file.basename[0] = '.';
    file.dir_ent.standard_8_3_format.attributes = 0x10;
    
    fat_listAll(&fat, &file);

    fat_file_destruct(&file);
    fat32_teardown(&fat);

    return 0;
}

// struct file_node_s
// {
//     char *dirname, *basename;
//     uint32_t cluster;
    
//     file_node_t *files;
//     int n_files;
// };

// void file_node_destruct(file_node_t *root)
// {
//     free(root->dirname);
//     free(root->basename);

//     free(root->files);
// }

void fat_listAll(const fat_t *p_fat, fat_file_t *p_file)
{
    // Print current filename
    if(!fat_file_is_dot_entry(p_file))
        printf("%s\n", p_file->basename);
    // Return if it is not directory
    if(!fat_file_is_directory(p_file))
        return;
    // Store the cluster content
    // meanwhile for each file or directory, call this function agoin
    uint32_t cluster = p_file->beg_marker;
    fat_file_t file_current;
    fat_direntry_t *buffer;
    int i_dir_table;
    const dir_tables_in_cluster = p_fat->bs->bytes_per_sector * p_fat->bs->sectors_per_cluster / 32;
    do
    {
        buffer = fat32_alloc_and_load_cluster(p_fat, cluster);
        for(i_dir_table=0; i_dir_table<dir_tables_in_cluster; ++i_dir_table)
        {
            if(fat_dir_entry_is_deleted(buffer+i_dir_table))
                continue;
            if(fat32_load_dir_table_and_return_true_if_end_of_chain(buffer+i_dir_table, &file_current))
            {
                fat_listAll(p_fat, &file_current);
                fat_file_destruct(&file_current);
            }
        }
        free(buffer);
        cluster = fat32_next_cluster(p_fat, cluster);
    } while(!fat32_end_of_cluster_chain(cluster));
}

// void file_node_listAll(file_node_t *p_node)
// {
//     if(!is_dot_entry(p_node->basename))
//         printf("%s/%s", p_node->dirname, p_node->basename);

//     int i;
//     for(i=0; i<p_node->n_files; ++i)
//     {
//         file_node_listAll(p_node->files);
//     }
// }
