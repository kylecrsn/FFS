#include "tests.h"
#include "alloc.h"
#include "sys_calls.h"

//Make directories for temp and output files, including their .keep files
void make_test_data()
{
	struct stat st = {0};
	FILE *fp;

	if (stat("./outputs", &st) == -1) 
	{
	    mkdir("./outputs", 0700);
	}
	if (stat("./temp", &st) == -1) 
	{
	    mkdir("./temp", 0700);
	}
	fp = fopen("./outputs/.keep", "ab+");
	fclose(fp);
	fp = fopen("./temp/.keep", "ab+");
	fclose(fp);
}

//Overwrite an existing file before writing/appending it
void clear_file(const char *fn)
{
	FILE *fp = fopen(fn, "w");
	fclose(fp);
}

//Dumps the contents of the disk to the specified file
void dump_disk_to_file(const char *fn, const char *fl)
{
	SBlock sblk;
	uint32_t i;
	FILE *fp = fopen(fn, fl);

	if(fp == NULL)
	{
		printf("Error while opening file %s\n", fn);
		return;
	}
	else
	{
		for(i = 0; i < _spblk.block_count; i++)
		{
			sblk = read_sblock(i);

			//Print block info
			fprintf(fp, "BLOCK %" PRIu32 " ", i);
			if(i == 0)
			{
				fprintf(fp, "(Super Block)");
			}
			else if(i >= _spblk.free_inode_list_offset && i < _spblk.free_data_list_offset)
			{
				fprintf(fp, "(Free Inode Block %" PRIu32 ")", i - _spblk.free_inode_list_offset);
			}
			else if(i >= _spblk.free_data_list_offset && i < _spblk.inode_list_offset)
			{
				fprintf(fp, "(Free Data Block %" PRIu32 ")", i - _spblk.free_data_list_offset);
			}
			else if(i >= _spblk.inode_list_offset && i < _spblk.data_list_offset)
			{
				fprintf(fp, "(Inode Block %" PRIu32 ")", i - _spblk.inode_list_offset);
			}
			else if(i >= _spblk.data_list_offset && i < _spblk.block_count)
			{
				fprintf(fp, "(Data Block %" PRIu32 ")", i - _spblk.data_list_offset);
			}
			fprintf(fp, "\n\n");

			print_sblock(fp, sblk);
		}
	}

	fclose(fp);
}

//Allocate inodes, write to disk, read back and compare for correctness
void comp_inodes(const char* fn, const char *fl)
{
	DBlock dblk;
	uint32_t i, comp, node_count = (2 * _spblk.block_size) / _spblk.inode_size;
	Inode init_nodes[node_count], disk_nodes[node_count];
	uint8_t uid[7] = "TestUid";
	FILE *fp = fopen(fn, fl);

	if(fp == NULL)
	{
		printf("Error while opening file %s\n", fn);
		return;
	}
	else
	{
		//Allocate and write about two blocks worth of inodes
		for(i = 0; i < node_count; i++)
		{
			init_nodes[i] = read_inode(allocate_inode());

			memcpy(init_nodes[i].uid, uid, sizeof(uid));
			init_nodes[i].file_size = rand();
			init_nodes[i].permissions = rand() % 777;
			init_nodes[i].link_count = rand() % 200;
			init_nodes[i].f_large = rand() % 2;

			write_inode(init_nodes[i]);
		}

		dblk = read_dblock(_spblk.inode_list_offset);

		//Read inodes back in from disk and compare
		for(i = 0; i < node_count; i++)
		{
			disk_nodes[i] = read_inode(i);

			comp = memcmp(&init_nodes[i], &disk_nodes[i], _spblk.inode_size);

			if(comp < 0 || comp > 0)
			{
				fprintf(fp, "Inconsistency with Inode Block %d:\n\n", i);
				print_dblock(fp, dblk);
				fclose(fp);
				return;
			}
		}
	}

	fprintf(fp, "Inode Comparison Passed\n\n");
	fclose(fp);
}

//Allocate data blocks, write to disk, read back and compare for correctness
void comp_data_blocks(const char* fn, const char *fl)
{
	uint32_t i, val32, comp, byte_choice, seg_size = 20 * _spblk.block_size;
	uint16_t val16;
	uint8_t val8, data_segment[seg_size];
	FILE *fp = fopen(fn, fl);

	memset(&data_segment, 0, seg_size);
	byte_choice = 0;

	if(fp == NULL)
	{
		printf("Error while opening file %s\n", fn);
		return;
	}
	else
	{
		//Allocate and write a data segment with 1, 2, and 4 byte values
		for(i = 0; i < seg_size;)
		{
			if(i + 4 >= seg_size)
			{
				break;
			}

			switch(byte_choice)
			{
				case 0:
				{
					val8 = rand() % (uint8_t)pow(2, 8);
					memcpy(&data_segment[i], &val8, sizeof(val8));
					i += sizeof(val8);
					byte_choice = 1;
				}
				case 1:
				{
					val16 = rand() % (uint16_t)pow(2, 16);
					memcpy(&data_segment[i], &val16, sizeof(val16));
					i += sizeof(val16);
					byte_choice = 2;
				}
				case 2:
				{
					val32 = rand() % (uint32_t)pow(2, 32);
					memcpy(&data_segment[i], &val32, sizeof(val32));
					i += sizeof(val32);
					byte_choice = 0;
				}
			}
		}

		//Copy the data segment into SBlocks and write them out to data blocks
		for(i = 0; i < seg_size / _spblk.block_size; i++)
		{
			SBlock sblk;
			uint32_t block_num = allocate_data_block();
			memcpy(&sblk.buffer, &data_segment[i * _spblk.block_size], _spblk.block_size);
			write_data_block(block_num, sblk);
		}

		//Read data blocks back in from disk and compare
		for(i = 0; i < seg_size / _spblk.block_size; i++)
		{
			SBlock sblk = read_data_block(i + 1);

			comp = memcmp(&data_segment[i * _spblk.block_size], &sblk.buffer, _spblk.block_size);

			if(comp < 0 || comp > 0)
			{
				fprintf(fp, "Inconsistency with Data Block %d:\n\n", i + 1);
				print_sblock(fp, sblk);
				fclose(fp);
				return;
			}
		}
	}

	fprintf(fp, "Data Block Comparison Passed\n\n");
	fclose(fp);
}

//Print out the contents of an inode in a readable format
void print_inode_data(const char* fn, const char *fl, Inode node)
{
	uint32_t i, inode_block_number, disk_block_number;
	FILE *fp;
	
	if(strcmp(fn, "stdout") == 0)
	{
		fp = stdout;
	}
	else
	{
		fp = fopen(fn, fl);
	}
	
	if(fp == NULL)
	{
		printf("Error while opening file %s\n", fn);
		return;
	}
	else
	{
		inode_block_number = (uint32_t)floor((double)(node.i_number * _spblk.inode_size) / _spblk.block_size);
		disk_block_number = _spblk.inode_list_offset + inode_block_number;
	
		fprintf(fp, "Inode (Disk Block %" PRIu32 " / Inode Block %" PRIu32 "):\n", disk_block_number, inode_block_number);
		fprintf(fp, "Number: %" PRIu32 "\n", node.i_number);
		for(i = 0; i < D_BLOCKS; i++)
		{
			fprintf(fp, "Direct Block %" PRIu32 ": %" PRIu32 "\n", i, node.d_blocks[i]);
		}
		for(i = 0; i < FI_BLOCKS; i++)
		{
			fprintf(fp, "Firstly Indirect Block %" PRIu32 ": %" PRIu32 "\n", i, node.fi_blocks[i]);
		}
		for(i = 0; i < SI_BLOCKS; i++)
		{
			fprintf(fp, "Secondly Indirect Block %" PRIu32 ": %" PRIu32 "\n", i, node.si_blocks[i]);
		}
		fprintf(fp, "User ID: ");
		for(i = 0; i < sizeof(node.uid); i++)
		{
			fprintf(fp, "%c", node.uid[i]);
		}
		fprintf(fp, "\n");
		fprintf(fp, "Group ID: ");
		for(i = 0; i < sizeof(node.gid); i++)
		{
			fprintf(fp, "%c", node.gid[i]);
		}
		fprintf(fp, "\n");
		fprintf(fp, "Access Time: %s", asctime(localtime(&node.access_time)));
		fprintf(fp, "Change Time: %s", asctime(localtime(&node.change_time)));
		fprintf(fp, "Modify Time: %s", asctime(localtime(&node.modify_time)));
		fprintf(fp, "File Size: %" PRIu64 "\n", node.file_size);
		fprintf(fp, "Permissions: %" PRIu16 "\n", node.permissions);
		fprintf(fp, "Link Count: %" PRIu16 "\n", node.link_count);
		fprintf(fp, "Type: %" PRIu32 "\n", node.f_type);
		fprintf(fp, "Large: %" PRIu32 "\n", node.f_large);
		fprintf(fp, "\n\n");
		if(fp != stdout) {
			fclose(fp);
		}
	}
}

void test_mkdir_bad_permissions() {
	uint8_t uid[255];
	uint8_t gid[255];
	
	strcpy((char*)uid, "uid");
	strcpy((char*)gid, "gid");

	ffs_mkdir(100, 0, 0, "/blah", uid, gid);
	uint32_t ret = ffs_mkdir(777, 0, 0, "/blah/hi", uid, gid);

	switch(ret) {
		case -1:
			printf("Test test_mkdir_bad_permissions Passed\n");
			break;
		default:
			printf("Test test_mkdir_bad_permissions Failed\n");
	}
}

void test_mkdir() {
	uint8_t uid[255];
	uint8_t gid[255];
	uint16_t permissions = 102;
	
	strcpy((char*)uid, "uid");
	strcpy((char*)gid, "gid");

	printf("\nCalling ffs_mkdir on /foo...\n");
	uint32_t ret = ffs_mkdir(permissions, 0, 0, "/foo", uid, gid);
	printf("\nDirectory /foo Created Successfully\n");
	printf("\nReading /foo into Inode struct using read_inode_directory\n");
	Inode newDir = read_inode_directory("/foo");
	
	//something went wrong with read_inode_directory
	if(newDir.permissions != permissions) {
		printf("\nSomething went wrong with read Inode Directory!\n");
		ret = -1;
	}
	
	Inode root = read_inode(0);
	printf("\n-----Root Inode-----\n");
	print_inode_data("stdout", "w", root);

	printf("\n-----New Directory-----\n");
	print_inode_data("stdout", "w", newDir);

	switch(ret) {
		case 0:
			printf("Test test_mkdir Passed with flying colors\n");
			break;
		default:
			printf("Test test_mkdir Failed with output %d\n", ret);
	}
}

void test_mkdir_multiple_directories() {
	uint8_t uid[255];
	uint8_t gid[255];
	uint16_t permissions = 102;
	
	strcpy((char*)uid, "uid");
	strcpy((char*)gid, "gid");
	
	ffs_mkdir(permissions, 0, 0, "/", uid, gid);
	
	ffs_mkdir(permissions, 0, 0, "/foo", uid, gid);
	
	ffs_mkdir(permissions, 0, 0, "/bar", uid, gid);
	
	ffs_mkdir(permissions, 0, 0, "/foo/leaf", uid, gid);
	
	ffs_mkdir(permissions, 0, 0, "/bar/apple/seed/nut", uid, gid);
	
	ffs_mkdir(permissions, 0, 0, "/bar/apple/seed/tree", uid, gid);
	
	print_key_vals(read_inode_directory("/").i_number);
	print_key_vals(read_inode_directory("/foo").i_number);
	print_key_vals(read_inode_directory("/foo/leaf").i_number);
	print_key_vals(read_inode_directory("/bar").i_number);
	print_key_vals(read_inode_directory("/bar/apple").i_number);
	print_key_vals(read_inode_directory("/bar/apple/seed").i_number);
	print_key_vals(read_inode_directory("/bar/apple/seed/nut").i_number);
	print_key_vals(read_inode_directory("/bar/apple/seed/tree").i_number);
}