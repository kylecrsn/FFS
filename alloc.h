#ifndef ALLOC_H
#define ALLOC_H

#include "global.h"

uint32_t allocate_inode();
void free_inode(uint32_t inode_number);
Inode read_inode(uint32_t inode_number);
void write_inode(Inode node);
uint32_t allocate_data_block();
void free_data_block(uint32_t block_number);
SBlock read_data_block(uint32_t block_number);
void write_data_block(uint32_t block_number, SBlock sblk);
void free_d_blocks(Inode node);
void free_fi_blocks(Inode node);
void free_si_blocks(Inode node);

#endif