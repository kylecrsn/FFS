#ifndef GLOBAL_H
#define GLOBAL_H

//Libraries available for access
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

//Macros
#define BITS_PER_BYTE 8
#define BLOCK_NUMBER 4
#define BLOCK_SIZE 1024
#define D_BLOCKS 8
#define FI_BLOCKS 3
#define SI_BLOCKS 1
#define BLOCK_COUNT (D_BLOCKS + (FI_BLOCKS * (BLOCK_SIZE / BLOCK_NUMBER)) + (SI_BLOCKS * (uint64_t)pow((BLOCK_SIZE / BLOCK_NUMBER), 2)))
#define USER_READ 0
#define USER_WRITE 1
#define USER_EXECUTE 2
#define MAX_PATH_LENGTH 4096

//Structs
typedef struct super_block {
	uint32_t bits_per_byte;
	uint32_t block_number;
	uint32_t block_size;
	uint32_t key_val_size;
	uint32_t block_count;
	uint32_t inode_blk_count;
	uint32_t inode_size;
	uint32_t inode_count;
	uint32_t free_inode_blk_count;
	uint32_t free_data_blk_count;
	uint32_t data_blk_count;
	uint32_t free_inode_list_offset;
	uint32_t free_data_list_offset;
	uint32_t inode_list_offset;
	uint32_t data_list_offset;
	uint32_t free_inode_byte_count;
	uint32_t free_data_byte_count;
} SuperBlock;

typedef struct inode {
	uint32_t i_number;
	uint32_t d_blocks[D_BLOCKS];
	uint32_t fi_blocks[FI_BLOCKS];
	uint32_t si_blocks[SI_BLOCKS];
	uint8_t uid[255];
	uint8_t gid[255];
	time_t access_time;
	time_t change_time;
	time_t modify_time;
	uint64_t file_size;
	uint16_t permissions;
	uint16_t link_count;
	uint8_t f_type;
	uint8_t f_large;
} Inode;

typedef struct key_val {
	uint32_t val;
	uint8_t size;
	uint8_t key[255];
} KeyVal;

typedef struct single_block {
	uint8_t buffer[BLOCK_SIZE];
} SBlock;

typedef struct double_block {
	uint8_t buffer[2 * BLOCK_SIZE];
} DBlock;

//Global variables
uint8_t *_disk;
SuperBlock _spblk;
uint8_t _little_endian;
uint8_t *_current_directory;
uint8_t *_parent_directory;

//Global functions
void clean();
SBlock read_sblock(uint32_t index);
void write_sblock(uint32_t index, SBlock sblk);
void clear_sblock(SBlock sblk);
void print_sblock(FILE *fp, SBlock sblk);
DBlock read_dblock(uint32_t index);
void write_dblock(uint32_t index, DBlock dblk);
void clear_dblock(DBlock dblk);
void print_dblock(FILE *fp, DBlock dblk);
uint32_t find_free_bit(uint8_t val);

#endif