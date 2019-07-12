#include "global.h"

//Clean up any dynamically allocated variables
void clean()
{
	free(_disk);
}

//Read a single block of data from the disk
SBlock read_sblock(uint32_t index)
{
	SBlock sblk;

	memcpy(&sblk, &_disk[index * BLOCK_SIZE], BLOCK_SIZE);

	return sblk;
}

//Write a single block of data to the disk
void write_sblock(uint32_t index, SBlock sblk)
{
	memcpy(&_disk[index * BLOCK_SIZE], &sblk, BLOCK_SIZE);
}

//Clear the contents of a sblock
void clear_sblock(SBlock sblk)
{
	memset(&sblk, 0, BLOCK_SIZE);
}

//Print the contents of a specific SBlock to the given file
void print_sblock(FILE *fp, SBlock sblk)
{
	uint32_t i, j, bytes_per_line, prev_offset;

	bytes_per_line = 32;
	prev_offset = 0;

	for(i = 0; i < (BLOCK_SIZE / bytes_per_line); i++)
	{
		for(j = prev_offset; j < (prev_offset + bytes_per_line); j++)
		{
			fprintf(fp, "x%02X", sblk.buffer[j]);
		}
		fprintf(fp, "\n");
		prev_offset = j;
	}
	fprintf(fp, "\n");
}

//Read a double block of data from the disk
DBlock read_dblock(uint32_t index)
{
	DBlock dblk;

	memcpy(&dblk, &_disk[index * _spblk.block_size], 2 * BLOCK_SIZE);

	return dblk;
}

//Write a double block of data to the disk
void write_dblock(uint32_t index, DBlock dblk)
{
	memcpy(&_disk[index * _spblk.block_size], &dblk, 2 * BLOCK_SIZE);
}

//Clear the contents of a dblock
void clear_dblock(DBlock dblk)
{
	memset(&dblk, 0, BLOCK_SIZE * 2);
}

//Print the contents of a specific DBlock to the given file
void print_dblock(FILE *fp, DBlock dblk)
{
	uint32_t i, j, k, bytes_per_line, prev_offset;

	bytes_per_line = 32;
	prev_offset = 0;

	for(i = 0; i < 2; i++)
	{
		for(j = 0; j < (BLOCK_SIZE / bytes_per_line); j++)
		{
			for(k = prev_offset; k < (prev_offset + bytes_per_line); k++)
			{
				fprintf(fp, "x%02X", dblk.buffer[k + (i * BLOCK_SIZE)]);
			}
			fprintf(fp, "\n");
			prev_offset = k;
		}
		fprintf(fp, "\n");
	}
}

//Find a free flag bit in an 8-bit byte
uint32_t find_free_bit(uint8_t val)
{
	uint32_t pos = 8;

	switch(val)
	{
		case 255:
		{
			pos = 7;
			break;
		}
		case 127:
		{
			pos = 6;
			break;
		}
		case 63:
		{
			pos = 5;
			break;
		}
		case 31:
		{
			pos = 4;
			break;
		}
		case 15:
		{
			pos = 3;
			break;
		}
		case 7:
		{
			pos = 2;
			break;
		}
		case 3:
		{
			pos = 1;
			break;
		}
		case 1:
		{
			pos = 0;
			break;
		}
	}

	return pos;
}