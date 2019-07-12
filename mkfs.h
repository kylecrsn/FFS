#ifndef MKFS_H
#define MKFS_H

#include "global.h"

void mount();
void mkfs();
void init_super_block();
void init_free_list();
void init_inode_list();
void init_current_directory();
void allocate_root_inode();

#endif