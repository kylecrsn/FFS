#include "alloc.h"

//Handling the allocation, freeing, readng, and writing of inodes

//Finds the first available inode and set's its free flag
uint32_t allocate_inode() 
{
	Inode node;
	SBlock sblk;
	uint32_t i, j, bit_offset, inode_number, byte_count, valid_bytes;
	uint8_t val, val_set;

	//Increment through the free inode list to find a free inode number
	inode_number = 0;
	byte_count = 0;
	val_set = 0;
	for(i = _spblk.free_inode_list_offset; i < _spblk.free_inode_list_offset + _spblk.free_inode_blk_count; i++)
	{
		sblk = read_sblock(i);

		//Only search for block_size bytes in each block, unless its the last block, in
		//which case make sure to only search the bytes that actually correspond to inodes
		if(_spblk.free_inode_byte_count - byte_count >= _spblk.block_size)
		{
			valid_bytes = _spblk.block_size;
		}
		else
		{
			valid_bytes = _spblk.free_inode_byte_count - byte_count;
		}

		for(j = 0; j < valid_bytes; j++)
		{
			//Read in a byte from the free inode list
			memcpy(&val, &sblk.buffer[j], sizeof(val));

			//Check if it references a free inode
			if(val != 0)
			{
				bit_offset = find_free_bit(val);
				val -= pow(2, bit_offset);
				memcpy(&sblk.buffer[j], &val, sizeof(val));
				write_sblock(i, sblk);
				inode_number += (_spblk.bits_per_byte - 1) - bit_offset;
				val_set = 1;
				break;
			}

			//Increment by the size of bits/byte
			inode_number += _spblk.bits_per_byte;
		}

		if(val_set == 1)
		{
			break;
		}

		byte_count += _spblk.block_size;
	}

	//No inodes left in the system, return 0 which is allocated by default for
	//the root inode, thus can be used (carefully) as an error code
	if(val_set == 0)
	{
		return 0;
	}

	node = read_inode(inode_number);

	//Make any necessary updates to the metadata
	time(&node.access_time);
	time(&node.change_time);
	time(&node.modify_time);

	write_inode(node);

	return node.i_number;
}

//Resets the values of the inode and any associated data blocks
void free_inode(uint32_t inode_number) 
{
	SBlock sblk;
	Inode node;
	uint32_t block_offset, byte_offset, bit_offset;
	uint8_t val;
	
	//Reset the data of the inode and its data blocks
	node = read_inode(inode_number);
	free_d_blocks(node);
	if(node.f_large == 1)
	{
		free_fi_blocks(node);
		free_si_blocks(node);
	}
	memset(node.uid, 0, 255);
	memset(node.gid, 0, 255);
	time(&node.access_time);
	time(&node.change_time);
	time(&node.modify_time);
	node.file_size = 0;
	node.permissions = 777;
	node.link_count = 0;
	node.f_type = 0;
	node.f_large = 0;

	//Get the free list block and byte that corresponds to the inode and which bit of that byte it directly referes to
	block_offset = _spblk.free_inode_list_offset + (uint32_t)floor(((double) inode_number / _spblk.bits_per_byte) / _spblk.block_size);
	byte_offset = (uint32_t)floor(((double)inode_number / _spblk.bits_per_byte)) % _spblk.block_size;
	bit_offset = inode_number % _spblk.bits_per_byte;

	//Read in the byte value there, set the appropriate bit to 1 for free, and write it back out to disk
	sblk = read_sblock(block_offset);
	memcpy(&val, &sblk.buffer[byte_offset], sizeof(val));
	val += pow(2, ((_spblk.bits_per_byte - 1) - bit_offset));
	memcpy(&sblk.buffer[byte_offset], &val, sizeof(val));
	write_sblock(block_offset, sblk);

	write_inode(node);
}

//Reads inode from disk into an inode object
Inode read_inode(uint32_t inode_number) 
{
	DBlock dblk;
	Inode node;
	uint32_t i, inode_block, block_offset, read_offset = 0;

	//Setup the index and offset
	inode_block = _spblk.inode_list_offset + (uint32_t)floor((double)(inode_number * _spblk.inode_size) / _spblk.block_size);
	block_offset = (inode_number * _spblk.inode_size) % _spblk.block_size;

	//Read data off the disk into the inode object
	dblk = read_dblock(inode_block);

	memcpy(&node.i_number, &dblk.buffer[block_offset + read_offset], sizeof(node.i_number));
	read_offset += sizeof(node.i_number);
	for(i = 0; i < D_BLOCKS; i++)
	{
		memcpy(&node.d_blocks[i], &dblk.buffer[block_offset + read_offset], sizeof(node.d_blocks[i]));
		read_offset += sizeof(node.d_blocks[i]);
	}
	for(i = 0; i < FI_BLOCKS; i++)
	{
		memcpy(&node.fi_blocks[i], &dblk.buffer[block_offset + read_offset], sizeof(node.fi_blocks[i]));
		read_offset += sizeof(node.fi_blocks[i]);
	}
	for(i = 0; i < SI_BLOCKS; i++)
	{
		memcpy(&node.si_blocks[i], &dblk.buffer[block_offset + read_offset], sizeof(node.si_blocks[i]));
		read_offset += sizeof(node.si_blocks[i]);
	}
	memcpy(&node.uid, &dblk.buffer[block_offset + read_offset], sizeof(node.uid));
	read_offset += sizeof(node.uid);
	memcpy(&node.gid, &dblk.buffer[block_offset + read_offset], sizeof(node.gid));
	read_offset += sizeof(node.gid);
	memcpy(&node.access_time, &dblk.buffer[block_offset + read_offset], sizeof(node.access_time));
	read_offset += sizeof(node.access_time);
	time(&node.access_time);
	memcpy(&node.change_time, &dblk.buffer[block_offset + read_offset], sizeof(node.change_time));
	read_offset += sizeof(node.change_time);
	// time(&node.change_time);
	memcpy(&node.modify_time, &dblk.buffer[block_offset + read_offset], sizeof(node.modify_time));
	read_offset += sizeof(node.modify_time);
	memcpy(&node.file_size, &dblk.buffer[block_offset + read_offset], sizeof(node.file_size));
	read_offset += sizeof(node.file_size);
	memcpy(&node.permissions, &dblk.buffer[block_offset + read_offset], sizeof(node.permissions));
	read_offset += sizeof(node.permissions);
	memcpy(&node.link_count, &dblk.buffer[block_offset + read_offset], sizeof(node.link_count));
	read_offset += sizeof(node.link_count);
	memcpy(&node.f_type, &dblk.buffer[block_offset + read_offset], sizeof(node.f_type));
	read_offset += sizeof(node.f_type);
	memcpy(&node.f_large, &dblk.buffer[block_offset + read_offset], sizeof(node.f_large));
	read_offset += sizeof(node.f_large);

	write_dblock(inode_block, dblk);
	
	return node;
}

//Writes the inode object data to the disk
void write_inode(Inode node) 
{
	DBlock dblk;
	uint32_t i, inode_block, block_offset, write_offset = 0;

	//Setup the index and offset
	inode_block = _spblk.inode_list_offset + (uint32_t)floor((double)(node.i_number * _spblk.inode_size) / _spblk.block_size);
	block_offset = (node.i_number * _spblk.inode_size) % _spblk.block_size;

	//Write inode data out to disk
	dblk = read_dblock(inode_block);

	memcpy(&dblk.buffer[block_offset + write_offset], &node.i_number, sizeof(node.i_number));
	write_offset += sizeof(node.i_number);
	for(i = 0; i < D_BLOCKS; i++)
	{
		memcpy(&dblk.buffer[block_offset + write_offset], &node.d_blocks[i], sizeof(node.d_blocks[i]));
		write_offset += sizeof(node.d_blocks[i]);
	}
	for(i = 0; i < FI_BLOCKS; i++)
	{
		memcpy(&dblk.buffer[block_offset + write_offset], &node.fi_blocks[i], sizeof(node.fi_blocks[i]));
		write_offset += sizeof(node.fi_blocks[i]);
	}
	for(i = 0; i < SI_BLOCKS; i++)
	{
		memcpy(&dblk.buffer[block_offset + write_offset], &node.si_blocks[i], sizeof(node.si_blocks[i]));
		write_offset += sizeof(node.si_blocks[i]);
	}
	memcpy(&dblk.buffer[block_offset + write_offset], &node.uid, sizeof(node.uid));
	write_offset += sizeof(node.uid);
	memcpy(&dblk.buffer[block_offset + write_offset], &node.gid, sizeof(node.gid));
	write_offset += sizeof(node.gid);
	memcpy(&dblk.buffer[block_offset + write_offset], &node.access_time, sizeof(node.access_time));
	write_offset += sizeof(node.access_time);
	time(&node.change_time);
	memcpy(&dblk.buffer[block_offset + write_offset], &node.change_time, sizeof(node.change_time));
	write_offset += sizeof(node.change_time);
	time(&node.modify_time);
	memcpy(&dblk.buffer[block_offset + write_offset], &node.modify_time, sizeof(node.modify_time));
	write_offset += sizeof(node.modify_time);
	memcpy(&dblk.buffer[block_offset + write_offset], &node.file_size, sizeof(node.file_size));
	write_offset += sizeof(node.file_size);
	memcpy(&dblk.buffer[block_offset + write_offset], &node.permissions, sizeof(node.permissions));
	write_offset += sizeof(node.permissions);
	memcpy(&dblk.buffer[block_offset + write_offset], &node.link_count, sizeof(node.link_count));
	write_offset += sizeof(node.link_count);
	memcpy(&dblk.buffer[block_offset + write_offset], &node.f_type, sizeof(node.f_type));
	write_offset += sizeof(node.f_type);
	memcpy(&dblk.buffer[block_offset + write_offset], &node.f_large, sizeof(node.f_large));
	write_offset += sizeof(node.f_large);

	write_dblock(inode_block, dblk);
}


//Handling the allocation, freeing, reading, and writting of data blocks

//Finds the first available data block and sets its free flag
//Return the data block number
uint32_t allocate_data_block() 
{
	SBlock sblk;
	uint32_t i, j, bit_offset, block_number, byte_count, valid_bytes;
	uint8_t val, val_set;

	//Increment through the free data block list to find a free data block number
	block_number = 0;
	byte_count = 0;
	val_set = 0;
	for(i = _spblk.free_data_list_offset; i < _spblk.free_data_list_offset + _spblk.free_data_blk_count; i++)
	{
		sblk = read_sblock(i);

		//Only search for block_size bytes in each block, unless its the last block, in
		//which case make sure to only search the bytes that actually correspond to data blocks
		if(_spblk.free_data_byte_count - byte_count >= _spblk.block_size)
		{
			valid_bytes = _spblk.block_size;
		}
		else
		{
			valid_bytes = _spblk.free_data_byte_count - byte_count;
		}

		for(j = 0; j < valid_bytes; j++)
		{
			//Read in a byte from the free data block list
			memcpy(&val, &sblk.buffer[j], sizeof(val));

			//Check if it references a free data block
			if(val != 0)
			{
				bit_offset = find_free_bit(val);
				val -= pow(2, bit_offset);
				memcpy(&sblk.buffer[j], &val, sizeof(val));
				write_sblock(i, sblk);
				block_number += (_spblk.bits_per_byte - 1) - bit_offset;
				val_set = 1;
				break;
			}

			//Increment by the size of bits/byte
			block_number += _spblk.bits_per_byte;
		}

		if(val_set == 1)
		{
			break;
		}

		byte_count += _spblk.block_size;
	}

	//No data blocks left in the system, return 0 which is allocated by default for
	//disambiguous block references in inodes
	if(val_set == 0)
	{
		return 0;
	}

	return block_number;
}

//Resets the value of the specified data block, settings its flag to free
void free_data_block(uint32_t block_number) 
{
	SBlock sblk;
	uint32_t block_offset, byte_offset, bit_offset;
	uint8_t val;

	//Wipe the block's data
	sblk = read_sblock(_spblk.data_list_offset + block_number);
	memset(&sblk, 0, _spblk.block_size);
	write_sblock(_spblk.data_list_offset + block_number, sblk);

	//Get the free list byte that corresponds to the data block and which bit of that byte it directly referes to
	block_offset = _spblk.free_data_list_offset + (uint32_t)floor(((double) block_number / _spblk.bits_per_byte) / _spblk.block_size);
	byte_offset = (uint32_t)floor(((double)block_number / _spblk.bits_per_byte)) % _spblk.block_size;
	bit_offset = block_number % _spblk.bits_per_byte;

	//Read in the byte value there, set the appropriate bit to 1 for free, and write it back out to disk
	sblk = read_sblock(block_offset);
	memcpy(&val, &sblk.buffer[byte_offset], sizeof(val));
	val += pow(2, ((_spblk.bits_per_byte - 1) - bit_offset));
	memcpy(&sblk.buffer[byte_offset], &val, sizeof(val));
	write_sblock(block_offset, sblk);
}

//Reads the contents of a data block into a sblock
SBlock read_data_block(uint32_t block_number) 
{
	SBlock sblk;

	sblk = read_sblock(_spblk.data_list_offset + block_number);

	return sblk;
}

//Writes the contents of the given buffer to the specified data block
void write_data_block(uint32_t block_number, SBlock sblk) 
{
	write_sblock(_spblk.data_list_offset + block_number, sblk);
}

//Free data blocks in an inode's directly addressable blocks
void free_d_blocks(Inode node)
{
	uint32_t i;

	for(i = 0; i < D_BLOCKS; i++)
	{
		if(node.d_blocks[i] != 0)
		{
			free_data_block(node.d_blocks[i]);
			memset(&node.d_blocks[i], 0, _spblk.block_number);
		}
	}
}

//Free data blocks in an inode's first-indirectly addressable blocks
void free_fi_blocks(Inode node)
{
	SBlock sblk;
	uint32_t i, j, fi_block = 0;

	for(i = 0; i < FI_BLOCKS; i++)
	{
		if(node.fi_blocks[i] != 0)
		{
			sblk = read_sblock(node.fi_blocks[i]);
			for(j = 0; j < (_spblk.block_size / _spblk.block_number); j++)
			{
				memcpy(&fi_block, &sblk.buffer[j * _spblk.block_number], _spblk.block_number);
				if(fi_block != 0)
				{
					free_data_block(fi_block);
				}
			}
			free_data_block(node.fi_blocks[i]);
			memset(&node.fi_blocks[i], 0, _spblk.block_size);
		}
	}
}

//Free data blocks in an inode's second-indirectly addressable blocks
void free_si_blocks(Inode node)
{
	SBlock sblk_outer, sblk_inner;
	uint32_t i, j, k, si_block_outer = 0, si_block_inner = 0;

	for(i = 0; i < SI_BLOCKS; i++)
	{
		if(node.si_blocks[i] != 0)
		{
			sblk_outer = read_sblock(node.si_blocks[i]);
			for(j = 0; j < (_spblk.block_size / _spblk.block_number); j++)
			{
				memcpy(&si_block_outer, &sblk_outer.buffer[j * _spblk.block_number], _spblk.block_number);
				if(si_block_outer != 0)
				{
					sblk_inner = read_sblock(si_block_outer);
					for(k = 0; k < (_spblk.block_size / _spblk.block_number); k++)
					{
						memcpy(&si_block_inner, &sblk_inner.buffer[k * _spblk.block_number], _spblk.block_number);
						if(si_block_inner != 0)
						{
							free_data_block(si_block_inner);
						}
					}
					free_data_block(si_block_outer);
				}
			}
			free_data_block(node.si_blocks[i]);
			memset(&node.si_blocks[i], 0, _spblk.block_size);
		}
	}
}