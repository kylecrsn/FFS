#include "dir_alloc.h"

/*
	reads all KeyVal's from an inode given the inode number
	returns array of KeyVal structs
*/
KeyVal *read_key_val(uint32_t inode_number, uint32_t *kvals_length) {
	Inode node;
	KeyVal *kvals;
	SBlock sblk_i, sblk_j, sblk_k;
	uint32_t i, j, k, l, block_num, kv_index, kv_count;
	
	node = read_inode(inode_number);
	kv_count = node.file_size / _spblk.key_val_size;
	*kvals_length = kv_count;
	kvals = malloc(kv_count * sizeof(KeyVal));
	kv_index = 0;
	
	for(i = 0; i < D_BLOCKS; i++)
	{
		//Check if all of the key-value pairs have been found
		if(kv_index == kv_count)
		{
			return kvals;
		}
		
		sblk_i = read_data_block(node.d_blocks[i]);
		for(j = 0; j < (uint32_t)floor((double)_spblk.block_size / _spblk.key_val_size); j++)
		{
			memcpy(&kvals[kv_index].size, &sblk_i.buffer[(j * _spblk.key_val_size) + sizeof(kvals[kv_index].val)], sizeof(kvals[kv_index].size));
			//Check if the key-value pair is allocated
			if(kvals[kv_index].size > 0)
			{
				memcpy(&kvals[kv_index].val, &sblk_i.buffer[(j * _spblk.key_val_size)], sizeof(kvals[kv_index].val));
				memcpy(&kvals[kv_index].key, &sblk_i.buffer[(j * _spblk.key_val_size) + sizeof(kvals[kv_index].val) + sizeof(kvals[kv_index].size)], sizeof(kvals[kv_index].key));
				kv_index++;
			}
		}
	}
	for(i = 0; i < FI_BLOCKS; i++)
	{
		//Check if all of the key-value pairs have been found
		if(kv_index == kv_count)
		{
			return kvals;
		}
		
		sblk_i = read_data_block(node.fi_blocks[i]);
		for(j = 0; j < _spblk.block_size / _spblk.block_number; j++)
		{
			memcpy(&block_num, &sblk_i.buffer[j * _spblk.block_number], sizeof(block_num));
			if(kv_index == kv_count)
			{
				return kvals;
			}
			
			sblk_j = read_data_block(block_num);
			for(k = 0; k < (uint32_t)floor((double)_spblk.block_size / _spblk.key_val_size); k++)
			{
				memcpy(&kvals[kv_index].size, &sblk_j.buffer[(k * _spblk.key_val_size) + sizeof(kvals[kv_index].val)], sizeof(kvals[kv_index].size));
				//Check if the kay-val pair is allocated
				if(kvals[kv_index].size > 0)
				{
					memcpy(&kvals[kv_index].val, &sblk_j.buffer[(j * _spblk.key_val_size)], sizeof(kvals[kv_index].val));
					memcpy(&kvals[kv_index].key, &sblk_j.buffer[(j * _spblk.key_val_size) + sizeof(kvals[kv_index].val) + sizeof(kvals[kv_index].size)], sizeof(kvals[kv_index].key));
					kv_index++;
				}
			}
		}
	}
	for(i = 0; i < SI_BLOCKS; i++)
	{
		//Check if all of the key-value pairs have been found
		if(kv_index == kv_count)
		{
			return kvals;
		}
		
		sblk_i = read_data_block(node.si_blocks[i]);
		for(j = 0; j < _spblk.block_size / _spblk.block_number; j++)
		{
			memcpy(&block_num, &sblk_i.buffer[j * _spblk.block_number], sizeof(block_num));
			if(kv_index == kv_count)
			{
				return kvals;
			}
			
			sblk_j = read_data_block(block_num);
			for(k = 0; k < _spblk.block_size / _spblk.block_number; k++)
			{
				memcpy(&block_num, &sblk_j.buffer[k * _spblk.block_number], sizeof(block_num));
				if(kv_index == kv_count)
				{
					return kvals;
				}
				
				sblk_k = read_data_block(block_num);
				for(l = 0; l < (uint32_t)floor((double)_spblk.block_size / _spblk.key_val_size); l++)
				{
					memcpy(&kvals[kv_index].size, &sblk_k.buffer[(l * _spblk.key_val_size) + sizeof(kvals[kv_index].val)], sizeof(kvals[kv_index].size));
					//Check if the kay-val pair is allocated
					if(kvals[kv_index].size > 0)
					{
						memcpy(&kvals[kv_index].val, &sblk_k.buffer[(j * _spblk.key_val_size)], sizeof(kvals[kv_index].val));
						memcpy(&kvals[kv_index].key, &sblk_k.buffer[(j * _spblk.key_val_size) + sizeof(kvals[kv_index].val) + sizeof(kvals[kv_index].size)], sizeof(kvals[kv_index].key));
						kv_index++;
					}
				}
			}
		}
	}

	return kvals;
}

/*
	writes a KeyVal to an inode_number
	returns 0 on error
	//What does it return on no error?
*/
uint32_t write_key_val(KeyVal kval, uint32_t inode_number) {
	Inode node;
	SBlock sblk_i, sblk_j, sblk_k;
	uint32_t i, j, k, l, block_num, err, size_check;
	
	node = read_inode(inode_number);
	err = 0;
	
	for(i = 0; i < D_BLOCKS; i++)
	{
		//Check if the block is allocated
		if(node.d_blocks[i] == 0)
		{
			//Allocate a new data block, associate it with the inode, and update the inode on disk
			node.d_blocks[i] = allocate_data_block();
		}
		
		sblk_i = read_data_block(node.d_blocks[i]);
		for(j = 0; j < (uint32_t)floor((double)_spblk.block_size / _spblk.key_val_size); j++)
		{
			memcpy(&size_check, &sblk_i.buffer[(j * _spblk.key_val_size) + sizeof(kval.val)], sizeof(size_check));
			//Find a key-value pair that isn't allocated already
			if(size_check == 0)
			{
				memcpy(&sblk_i.buffer[(j * _spblk.key_val_size)], &kval.val, sizeof(kval.val));
				memcpy(&sblk_i.buffer[(j * _spblk.key_val_size) + sizeof(kval.val)], &kval.size, sizeof(kval.size));
				memcpy(&sblk_i.buffer[(j * _spblk.key_val_size) + sizeof(kval.val) + sizeof(kval.size)], &kval.key, sizeof(kval.key));
				write_data_block(node.d_blocks[i], sblk_i);
				node.file_size += _spblk.key_val_size;
				write_inode(node);
				return 1;
			}
		}
	}
	for(i = 0; i < FI_BLOCKS; i++)
	{
		//Check if the block is allocated
		if(node.fi_blocks[i] == 0)
		{
			//Allocate a new data block, associate it with the inode, and update the inode on disk
			node.fi_blocks[i] = allocate_data_block();
		}
		
		sblk_i = read_data_block(node.fi_blocks[i]);
		for(j = 0; j < _spblk.block_size / _spblk.block_number; j++)
		{
			memcpy(&block_num, &sblk_i.buffer[j * _spblk.block_number], sizeof(block_num));
			//Check if the block is allocated
			if(block_num == 0)
			{
				block_num = allocate_data_block();
				memcpy(&sblk_i.buffer[j * _spblk.block_number], &block_num, sizeof(_spblk.block_number));
				write_data_block(node.fi_blocks[i], sblk_i);
			}
			
			sblk_j = read_data_block(block_num);
			for(k = 0; k < (uint32_t)floor((double)_spblk.block_size / _spblk.key_val_size); k++)
			{
				memcpy(&size_check, &sblk_j.buffer[(k * _spblk.key_val_size) + sizeof(kval.val)], sizeof(size_check));
				//Find a key-value pair that isn't allocated already
				if(size_check == 0)
				{
					memcpy(&sblk_j.buffer[(k * _spblk.key_val_size)], &kval.val, sizeof(kval.val));
					memcpy(&sblk_j.buffer[(k * _spblk.key_val_size) + sizeof(kval.val)], &kval.size, sizeof(kval.size));
					memcpy(&sblk_j.buffer[(k * _spblk.key_val_size) + sizeof(kval.val) + sizeof(kval.size)], &kval.key, sizeof(kval.key));
					write_data_block(block_num, sblk_j);
					node.file_size += _spblk.key_val_size;
					write_inode(node);
					return 1;
				}
			}
		}
	}
	for(i = 0; i < SI_BLOCKS; i++)
	{
		//Check if the block is allocated
		if(node.si_blocks[i] == 0)
		{
			//Allocate a new data block, associate it with the inode, and update the inode on disk
			node.si_blocks[i] = allocate_data_block();
		}
		
		sblk_i = read_data_block(node.si_blocks[i]);
		for(j = 0; j < _spblk.block_size / _spblk.block_number; j++)
		{
			memcpy(&block_num, &sblk_i.buffer[j * _spblk.block_number], sizeof(block_num));
			//Check if the block is allocated
			if(block_num == 0)
			{
				block_num = allocate_data_block();
				memcpy(&sblk_i.buffer[j * _spblk.block_number], &block_num, sizeof(_spblk.block_number));
				write_data_block(node.si_blocks[i], sblk_i);
			}
			
			sblk_j = read_data_block(block_num);
			for(k = 0; k < _spblk.block_size / _spblk.block_number; k++)
			{
				memcpy(&block_num, &sblk_j.buffer[k * _spblk.block_number], sizeof(block_num));
				//Check if the block is allocated
				if(block_num == 0)
				{
					block_num = allocate_data_block();
					memcpy(&sblk_j.buffer[k * _spblk.block_number], &block_num, sizeof(_spblk.block_number));
					write_data_block(block_num, sblk_j);
				}
				
				sblk_k = read_data_block(block_num);
				for(l = 0; l < (uint32_t)floor((double)_spblk.block_size / _spblk.key_val_size); l++)
				{
					memcpy(&size_check, &sblk_k.buffer[(l * _spblk.key_val_size) + sizeof(kval.val)], sizeof(size_check));
					//Find a key-value pair that isn't allocated already
					if(size_check == 0)
					{
						memcpy(&sblk_k.buffer[(l * _spblk.key_val_size)], &kval.val, sizeof(kval.val));
						memcpy(&sblk_k.buffer[(l * _spblk.key_val_size) + sizeof(kval.val)], &kval.size, sizeof(kval.size));
						memcpy(&sblk_k.buffer[(l * _spblk.key_val_size) + sizeof(kval.val) + sizeof(kval.size)], &kval.key, sizeof(kval.key));
						write_data_block(block_num, sblk_k);
						node.file_size += _spblk.key_val_size;
						write_inode(node);
						return 1;
					}
				}
			}
		}
	}
	
	return err;
}

//Print out the values of all the key-value pairs in an array
void print_key_vals(uint32_t inode_number)
{
	uint32_t i, j, kvals_length;
	KeyVal *kvals = read_key_val(inode_number, &kvals_length);
	printf("-------------Inode %d KVALS------------------\n", inode_number);
	printf("Kvals length: %" PRIu32 "\n", kvals_length);
	printf("Size of Key Value array: %lu\n\n", sizeof(kvals));
	for(i = 0; i< kvals_length; i++)
	{
		printf("Key Value %" PRIu32 ":\n", i);
		printf("Inode Number Key: %" PRIu32 "\n", kvals[i].val);
		printf("Size: %" PRIu8 "\n", kvals[i].size);
		printf("Entry Value: " );
		for(j = 0; j < kvals[i].size; j++)
		{
			printf("%c", (char)kvals[i].key[j]);
		}
		printf("\n\n");
	}
	printf("---------------------------------------\n");
}

/* 
	adds path: new_directory with corresponding val:inode_number to destination_directory Inode
	returns 0 on error: error could be one or more of the following
		- no more inodes to allocate
		- destination_directory can't hold more directories within it
	and writes new inode out to disk
*/
uint32_t add_inode_directory(char *new_directory, Inode destination_directory) {
	printf("\n\nadd_inode_directory being called to create directory \"%s\" within Inode %d\n\n", new_directory, destination_directory.i_number);
	uint32_t inode_number;
	KeyVal kval;
	uint32_t err;
	inode_number = allocate_inode();
	if(inode_number == 0) {
		//some kind of error occurred allocating an inode
	} else {
		//create new inode for directory
		strcpy((char*)kval.key, new_directory);
		kval.val = inode_number;
		kval.size = strlen(new_directory);
		//attempt to write out "keyval" to destination directory inode
		err = write_key_val(kval, destination_directory.i_number);
		
		// print_key_vals(destination_directory.i_number);
		//If we can't write the key value pair for some reason, we need to free the inode
		//and set inode_number to zero to indicate error
		if(err == 0) {
			free_inode(inode_number);
			inode_number = 0;
		}
	}
	
	//return the new inode number we've created
	return inode_number;
}

//given directory path returns Inode corresponding to final directory
Inode read_inode_directory(char *full_p) {
	printf("\n\nReading Inode Directory %s\n\n", full_p);
	//Need to traverse to find current directory based on current path
	uint32_t len = strlen(full_p) + 1;
	char full_path[len];
	strcpy(full_path, full_p);
	char path[len];
	memset(path, 0, len);
	char *tok;
	int i = 0;
	int found = 0;
	KeyVal *kvals;
	uint32_t kvals_length = 0;
	uint32_t inode_number = 0;
	tok = strtok(full_path, "/");
	//if full_path is NOT equal to "/", begin traversal
	if(strcmp(full_path, "/")) {

		while(tok != NULL) {
			found = 0;
			//copy path name
			strcpy(path, tok);
			kvals = read_key_val(inode_number, &kvals_length);
	
			if(kvals_length == 0) {
				printf("\ndir_alloc.c: kvals struct list returned from read_key_val was empty!\n");
			}

			// print_key_vals(inode_number);

			//traverse through KeyVal struct
			for(i = 0; i < kvals_length; i++) {
				//if the directory is found within the KeyVal struct
				// printf("\n\n\n Checking key:  \"%s\"\n\n\n", (char*)kvals[i].key);
				if(!strcmp((char*)kvals[i].key, path)) {
					inode_number = kvals[i].val;
					found = 1;
					break;
				}
			}

			tok = strtok(NULL, "/");
		}
		
		if(!found) {
			//set some kind of error
			printf("\ndir_alloc.c: Directory %s was not found within inode %d!\n", path, inode_number);
		}
	}

	// free(kvals);

	return read_inode(inode_number);
}

void writeCurrentAndPreviousDirectory(uint32_t current_directory_number, uint32_t previous_directory_number) {
	KeyVal kval1, kval2;
	strcpy((char*)kval1.key, ".");
	strcpy((char*)kval2.key, "..");
	kval1.size = 1;
	kval2.size = 2;
	kval1.val = current_directory_number;
	kval2.val = previous_directory_number;
	write_key_val(kval1, current_directory_number);
	write_key_val(kval2, current_directory_number);
}


/*
	takes a path in any format e.g. ./foo/bar ../foo/bar/ ../.foo  ~/foo/bar etc
	and converts it to an absolute path
	and removes the final / on the path
*/
void relativeToAbsolutePath(char *path, char *absolute_path) {
	int amount;
	int relative = 0;
	
	if(strlen(path) != 1 && path[strlen(path) - 1] == '/') {
		//snip that shit off
		path[strlen(path) - 1] = '\0';
	}

	if(path[0] == '/') {
		strcpy(absolute_path, path);
	} else if(path[0] == '.') {
		if(path[1] == '.') {
			if(path[1] == '/') {
				//../path
				amount = 2;
				strcpy(absolute_path, (char*)_parent_directory);
				//if current directory is /,
				// concat / and path without ./
				// EX path = ../foo & curr_dir = "/"
				// / + foo
				if(!strcmp((char*)_parent_directory, "/")) {
					amount++;
				}
				strcat(absolute_path, path + amount);
			} else {
				//path is ..filename
				relative = 1;
			}
		} else if(path[1] == '/') {
			//relative path supplied
			//path is ./filename
			amount = 1;
			strcpy(absolute_path, (char*)_current_directory);
			//if current directory is /,
			// concat / and path without ./
			// EX path = ./foo & curr_dir = "/"
			// / + foo
			if(!strcmp((char*)_current_directory, "/")) {
				amount++;
			}
			strcat(absolute_path, path + amount);
		}
	} else if(path[0] == '~') {
		amount = 1;

		//replace this with what ~ is
		strcpy(absolute_path, "/User");
		strcat(absolute_path, path + amount);
	} else {
		relative = 1;
	}
	
	if(relative) {
		//relative path supplied
		strcpy(absolute_path, (char*)_current_directory);

		if(strcmp((char*)_current_directory, "/")) {
			strcat(absolute_path, "/");
		}
		strcat(absolute_path, path);
	}	
}

/*
	checks if the user has permission to read/write/execute within the target_inode
	returns 1 if they can access
	returns 0 if access is denied
*/
int checkUserPermissions(Inode target_inode, uint8_t uid[255], uint8_t gid[255], int access_type) {
	char buf[4];
	int allowed = 0;
	int user;
	int group;
	int global;
	int mask;
	
	switch(access_type) {
		case USER_READ:
			//read
			mask = 0b100;
			break;
		case USER_WRITE:
			//write
			mask = 0b010;
			break;
		default:
			//execute
			mask = 0b001;
	}
	sprintf(buf, "%d", target_inode.permissions);

	user = buf[0] - 48;
	group = buf[1] - 48;
	global = buf[2] - 48;
	// rwx rwx rwx
	// XXX XXX XXX
	//check if gid can write in current directory
	if(!strcmp((char*)target_inode.gid, (char*)gid)) {
		if((group & mask) > 0) {
			allowed = 1;
		}
	}
	//check if uid can write in directory
	if(!strcmp((char*)target_inode.uid, (char*)uid)) {
		if((user & mask) > 0) {
			allowed = 1;
		}
	}
	//check if all users can write in current directory
	if((global & mask) > 0) {
		allowed = 1;
	}
	
	return allowed;
}