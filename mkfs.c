#include "mkfs.h"
#include "alloc.h"
#include "tests.h"
#include "dir_alloc.h"
//Emulates mounting a disk; allocates space in memory for the "hard drive" buffer
void mount()
{
	_disk = (uint8_t *)malloc(sizeof(uint8_t) * (BLOCK_COUNT * BLOCK_SIZE));
	memset(_disk, 0, sizeof(uint8_t) * (BLOCK_COUNT * BLOCK_SIZE));
}

//Builds the file system within the newly created disk. This includes:
//- Initializing the super block and confirming the endianness
//- Initializing the free list
//- Initializing the ilist
//- Claim the 0th data block to distinguish unallocated data block numbers in inodes
void mkfs() {
	init_super_block();
	init_free_list();
	init_inode_list();
	init_current_directory();
	//Allocate the first data block as a "dummy block" to avoid confusion whether 
	//one of an inode's data blocks refers to data block 0 or is just unassigned
	allocate_data_block();
	
	//Allocate the first inode as the root diectory for the file system
	allocate_root_inode();
}

//Initializes all of the super block values
void init_super_block() {
	SBlock sblk = read_sblock(0);
	uint32_t sblk_offset = 0;
	uint8_t endian_l, endian_r;

	//How many bits per byte
	_spblk.bits_per_byte = BITS_PER_BYTE;
	memcpy(&sblk.buffer[sblk_offset], &_spblk.bits_per_byte, sizeof(_spblk.bits_per_byte));
	sblk_offset += sizeof(_spblk.bits_per_byte);

	//How many bytes it takes to represent the largest block number
	_spblk.block_number = BLOCK_NUMBER;
	memcpy(&sblk.buffer[sblk_offset], &_spblk.block_number, sizeof(_spblk.block_number));
	sblk_offset += sizeof(_spblk.block_number);

	//Determine whether the underlying hardware is big-endian or little-endian checking against the block number
	memcpy(&endian_l, &sblk.buffer[sblk_offset - sizeof(_spblk.block_number)], sizeof(endian_l));
	memcpy(&endian_r, &sblk.buffer[sblk_offset - sizeof(_spblk.block_number) + (sizeof(_spblk.block_number) - 1)], sizeof(endian_r));
	//Big-endian
	if(endian_l == 0 && endian_r == _spblk.block_number)
	{
		_little_endian = 0;
	}
	//Little-endian
	else
	{
		_little_endian = 1;
	}

	//Size of each addressable block
	_spblk.block_size = BLOCK_SIZE;
	memcpy(&sblk.buffer[sblk_offset], &_spblk.block_size, sizeof(_spblk.block_size));
	sblk_offset += sizeof(_spblk.block_size);

	//Size of each key-value pair for inode numbers-names
	KeyVal kval;
	_spblk.key_val_size += sizeof(kval.val);
	_spblk.key_val_size += sizeof(kval.size);
	_spblk.key_val_size += sizeof(kval.key);
	memcpy(&sblk.buffer[sblk_offset], &_spblk.key_val_size, sizeof(_spblk.key_val_size));
	sblk_offset += sizeof(_spblk.key_val_size);
	
	//Total Number of blocks
	_spblk.block_count = BLOCK_COUNT;
	memcpy(&sblk.buffer[sblk_offset], &_spblk.block_count, sizeof(_spblk.block_count));
	sblk_offset += sizeof(_spblk.block_count);

	//Calculate the number of inode blocks
	_spblk.inode_blk_count = (uint32_t)floor((double)_spblk.block_count * 0.1);
	memcpy(&sblk.buffer[sblk_offset], &_spblk.inode_blk_count, sizeof(_spblk.inode_blk_count));
	sblk_offset += sizeof(_spblk.inode_blk_count);

	//Number of bytes to represent one inode
	Inode node;
	_spblk.inode_size += sizeof(node.i_number);
	_spblk.inode_size += sizeof(node.d_blocks);
	_spblk.inode_size += sizeof(node.fi_blocks);
	_spblk.inode_size += sizeof(node.si_blocks);
	_spblk.inode_size += sizeof(node.uid);
	_spblk.inode_size += sizeof(node.gid);
	_spblk.inode_size += sizeof(node.access_time);
	_spblk.inode_size += sizeof(node.change_time);
	_spblk.inode_size += sizeof(node.modify_time);
	_spblk.inode_size += sizeof(node.file_size);
	_spblk.inode_size += sizeof(node.permissions);
	_spblk.inode_size += sizeof(node.link_count);
	_spblk.inode_size += sizeof(node.f_type);
	_spblk.inode_size += sizeof(node.f_large);
	memcpy(&sblk.buffer[sblk_offset], &_spblk.inode_size, sizeof(_spblk.inode_size));
	sblk_offset += sizeof(_spblk.inode_size);

	//Total number of inodes; account for the byte-size of an inode
	//not evenly fitting into the number of bytes in the inode list
	_spblk.inode_count = (uint32_t)floor((double)(_spblk.inode_blk_count * BLOCK_SIZE) / _spblk.inode_size);
	memcpy(&sblk.buffer[sblk_offset], &_spblk.inode_count, sizeof(_spblk.inode_count));
	sblk_offset += sizeof(_spblk.inode_count);

	//Calculate the number of blocks in the free inode list; take the ceil of each calculation
	//to make sure there is enough space to represent all of the inodes
	_spblk.free_inode_blk_count = (uint32_t)ceil((double)_spblk.inode_count / (_spblk.bits_per_byte * _spblk.block_size));
	memcpy(&sblk.buffer[sblk_offset], &_spblk.free_inode_blk_count, sizeof(_spblk.free_inode_blk_count));
	sblk_offset += sizeof(_spblk.free_inode_blk_count);

	//Calculate the number of blocks in the free data block list; take the ceil of each calculation
	//to make sure there is enough space to represent all of the data blocks
	_spblk.free_data_blk_count = (uint32_t)ceil((double)(_spblk.block_count - (1 + _spblk.inode_blk_count + _spblk.free_inode_blk_count)) / (_spblk.bits_per_byte * _spblk.block_size));
	memcpy(&sblk.buffer[sblk_offset], &_spblk.free_data_blk_count, sizeof(_spblk.free_data_blk_count));
	sblk_offset += sizeof(_spblk.free_data_blk_count);

	//Calculate the number of data blocks as the remainder of blocks
	_spblk.data_blk_count = _spblk.block_count - (1 + _spblk.inode_blk_count + _spblk.free_inode_blk_count + _spblk.free_data_blk_count);
	memcpy(&sblk.buffer[sblk_offset], &_spblk.data_blk_count, sizeof(_spblk.data_blk_count));
	sblk_offset += sizeof(_spblk.data_blk_count);

	//Offset to the list of free inode blocks
	_spblk.free_inode_list_offset = 1;
	memcpy(&sblk.buffer[sblk_offset], &_spblk.free_inode_list_offset, sizeof(_spblk.free_inode_list_offset));
	sblk_offset += sizeof(_spblk.free_inode_list_offset);

	//Offset to the list of free data blocks
	_spblk.free_data_list_offset = _spblk.free_inode_list_offset + _spblk.free_inode_blk_count;
	memcpy(&sblk.buffer[sblk_offset], &_spblk.free_data_list_offset, sizeof(_spblk.free_data_list_offset));
	sblk_offset += sizeof(_spblk.free_data_list_offset);

	//Offset to the inode list blocks
	_spblk.inode_list_offset = _spblk.free_data_list_offset + _spblk.free_data_blk_count;
	memcpy(&sblk.buffer[sblk_offset], &_spblk.inode_list_offset, sizeof(_spblk.inode_list_offset));
	sblk_offset += sizeof(_spblk.inode_list_offset);

	//Offset to the data list blocks
	_spblk.data_list_offset = _spblk.inode_list_offset + _spblk.inode_blk_count;
	memcpy(&sblk.buffer[sblk_offset], &_spblk.data_list_offset, sizeof(_spblk.data_list_offset));
	sblk_offset += sizeof(_spblk.data_list_offset);

	//Calculate the number of valid free inode list bytes
	_spblk.free_inode_byte_count = (uint32_t)ceil((double)_spblk.inode_blk_count / _spblk.bits_per_byte);
	memcpy(&sblk.buffer[sblk_offset], &_spblk.free_inode_byte_count, sizeof(_spblk.free_inode_byte_count));
	sblk_offset += sizeof(_spblk.free_inode_byte_count);

	//Calculate the number of valid free data list bytes
	_spblk.free_data_byte_count = (uint32_t)ceil((double)_spblk.data_blk_count / _spblk.bits_per_byte);
	memcpy(&sblk.buffer[sblk_offset], &_spblk.free_data_byte_count, sizeof(_spblk.free_data_byte_count));
	sblk_offset += sizeof(_spblk.free_data_byte_count);

	write_sblock(0, sblk);
}

//Setup the free lists so all flags are 1
void init_free_list()
{
	uint32_t i;

	for(i = _spblk.free_inode_list_offset; i < _spblk.free_inode_list_offset + _spblk.free_inode_blk_count + _spblk.free_data_blk_count; i++)
	{
		SBlock sblk = read_sblock(i);
		memset(&sblk.buffer, 255, _spblk.block_size);
		write_sblock(i, sblk);
	}
}

//Sets up all of the inodes contiguously in memory
void init_inode_list() 
{
	Inode node;
	uint32_t i, j, inode_num, dblk_offset;

	inode_num = 0;
	dblk_offset = 0;
	//Increment over all of the inode list blocks
	for(i = _spblk.inode_list_offset; i < _spblk.inode_list_offset + _spblk.inode_blk_count; i++)
	{
		//Read the ith and (i + 1)th blocks into dblk.buffer
		DBlock dblk = read_dblock(i);

		//Skip when writing the last inode block since it'll already be full and the dblk's
		//second block is actually the first data block, don't want to write into it
		if(i != _spblk.inode_list_offset + _spblk.inode_blk_count - 1)
		{
			//Make sure there is space to write only full inodes to the double block buffer
			//by checking the current offset + the size of the inode you're about to write
			while(dblk_offset + _spblk.inode_size < 2 * _spblk.block_size)
			{
				//Initialize the inode's values to default/null
				node.i_number = inode_num;
				memcpy(&dblk.buffer[dblk_offset], &node.i_number, sizeof(node.i_number));
				dblk_offset += sizeof(node.i_number);

				for(j = 0; j < D_BLOCKS; j++)
				{
					node.d_blocks[j] = 0;
					memcpy(&dblk.buffer[dblk_offset], &node.d_blocks[j], sizeof(node.d_blocks[j]));
					dblk_offset += sizeof(node.d_blocks[j]);
				}
				for(j = 0; j < FI_BLOCKS; j++)
				{
					node.fi_blocks[j] = 0;
					memcpy(&dblk.buffer[dblk_offset], &node.fi_blocks[j], sizeof(node.fi_blocks[j]));
					dblk_offset += sizeof(node.fi_blocks[j]);
				}
				for(j = 0; j < SI_BLOCKS; j++)
				{
					node.si_blocks[j] = 0;
					memcpy(&dblk.buffer[dblk_offset], &node.si_blocks[j], sizeof(node.si_blocks[j]));
					dblk_offset += sizeof(node.si_blocks[j]);
				}

				memset(node.uid, 0, 256);
				memcpy(&dblk.buffer[dblk_offset], &node.uid, sizeof(node.uid));
				dblk_offset += sizeof(node.uid);

				memset(node.gid, 0, 256);
				memcpy(&dblk.buffer[dblk_offset], &node.gid, sizeof(node.gid));
				dblk_offset += sizeof(node.gid);

				time(&node.access_time);
				memcpy(&dblk.buffer[dblk_offset], &node.access_time, sizeof(node.access_time));
				dblk_offset += sizeof(node.access_time);

				time(&node.change_time);
				memcpy(&dblk.buffer[dblk_offset], &node.change_time, sizeof(node.change_time));
				dblk_offset += sizeof(node.change_time);

				time(&node.modify_time);
				memcpy(&dblk.buffer[dblk_offset], &node.modify_time, sizeof(node.modify_time));
				dblk_offset += sizeof(node.modify_time);

				node.file_size = 0;
				memcpy(&dblk.buffer[dblk_offset], &node.file_size, sizeof(node.file_size));
				dblk_offset += sizeof(node.file_size);

				node.permissions = 777;
				memcpy(&dblk.buffer[dblk_offset], &node.permissions, sizeof(node.permissions));
				dblk_offset += sizeof(node.permissions);

				node.link_count = 0;
				memcpy(&dblk.buffer[dblk_offset], &node.link_count, sizeof(node.link_count));
				dblk_offset += sizeof(node.link_count);

				node.f_type = 0;
				memcpy(&dblk.buffer[dblk_offset], &node.f_type, sizeof(node.f_type));
				dblk_offset += sizeof(node.f_type);

				node.f_large = 0;
				memcpy(&dblk.buffer[dblk_offset], &node.f_large, sizeof(node.f_large));
				dblk_offset += sizeof(node.f_large);

				inode_num++;
			}

			//Write out the dblk; the next read will read back in the latter of the two blocks
			//currently within dblk, that way it can fill the space at the end of it. Thus, with
			//each write, you write one complete block and one incomplete block; the next time you
			//read, you'll read back in the previously incomplete block, complete it, and fill the
			//next block incompletely, repeating until you've reached the end
			write_dblock(i, dblk);

			//Start the next chunk of inode writes from the correct index in the latter block
			dblk_offset -= _spblk.block_size;
		}
	}
}

void init_current_directory() {
	_current_directory = (uint8_t *)malloc(sizeof(uint8_t) * MAX_PATH_LENGTH);
	_parent_directory = (uint8_t *)malloc(sizeof(uint8_t) * MAX_PATH_LENGTH);
	
	memset(_current_directory, 0, sizeof(uint8_t) * MAX_PATH_LENGTH);
	memset(_parent_directory, 0, sizeof(uint8_t) * MAX_PATH_LENGTH);
	
	strcpy((char*)_current_directory, "/");
	strcpy((char*)_parent_directory, "/");
}

void allocate_root_inode() {
	uint32_t inode_number = allocate_inode();
	if(inode_number != 0) {
		printf("Error, root inode must have i_number of zero\n");
	}
	
	writeCurrentAndPreviousDirectory(inode_number, inode_number);
}