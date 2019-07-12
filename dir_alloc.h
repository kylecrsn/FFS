#ifndef DIR_ALLOC_H
#define DIR_ALLOC_H

#include "alloc.h"

uint32_t add_inode_directory(char *new_directory, Inode destination_directory);
Inode read_inode_directory(char *full_path);
uint32_t write_key_val(KeyVal key_val, uint32_t inode_number);
KeyVal *read_key_val(uint32_t inode_number, uint32_t *kvals_length);
void print_key_vals(uint32_t inode_number);
void writeCurrentAndPreviousDirectory(uint32_t current_directory_number, uint32_t previous_directory_number);
int checkUserPermissions(Inode target_inode, uint8_t uid[255], uint8_t gid[255], int access_type);
void relativeToAbsolutePath(char *path, char *absolute_path);

#endif