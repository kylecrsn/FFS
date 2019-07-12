#include "sys_calls.h"

uint32_t ffs_mkdir(uint16_t mode, int p_flag, int v_flag, char* path, uint8_t uid[255], uint8_t gid[255])
{
	if(!strcmp(path, "/")) {
		printf("mkdir: /: Is a directory\n");
		return -1;
	} else if(!strcmp(path, "~")) {
		printf("mkdir: ~: Is a directory\n");
		return -1;
	}
	int err = 0;
	int allowed = 0;
	Inode new_inode;
	Inode target_inode;
	uint32_t inode_number;
	char absolute_path[MAX_PATH_LENGTH];
	memset(absolute_path, 0, MAX_PATH_LENGTH);
	
	relativeToAbsolutePath(path, absolute_path);
	
	char directory_path[strlen(absolute_path) + 1];
	char *name = strrchr(absolute_path, '/');
	char directory_name[strlen(name)];
	int path_length = strlen(absolute_path) - strlen(name);
	
	if(!path_length) {
		path_length = 1;
	}
	
	//memset both directory path and name to zero;
	memset(directory_path, 0, path_length + 1);
	memset(directory_name, 0, strlen(name));

	//new directory name does not need preceding slash
	name++;

	strncpy(directory_path, path, path_length);
	strcpy(directory_name, name);
	// printf("hi\n%s\n%s\n%s\n", directory_path, path, absolute_path);
	target_inode = read_inode_directory(directory_path);
	
	//if directory_name is NOT / and we returned inode 0
	//it was because the directory was not found
	if(strcmp(directory_path, "/") && target_inode.i_number == 0) {
		printf("mkdir: /%s: No such file or directory\n", path);
		return -1;
	}
	
	allowed = checkUserPermissions(target_inode, uid, gid, USER_WRITE);

	if(allowed) {
		/*
			add_inode_directory is expecting path to be the name of the new directory only
		*/
		
		inode_number = add_inode_directory(directory_name, target_inode);
		
		if(inode_number == 0) {
			//something went wrong trying allocate new inode.. out of directory entries..etc
			err = -1;
		} else {
			
			writeCurrentAndPreviousDirectory(inode_number, target_inode.i_number);
			
			//set all proper things for node
			new_inode = read_inode(inode_number);
			
			//set file type to directory
			new_inode.f_type = 1;
			
			//If permissions flag set
			new_inode.permissions = mode ? mode : 777;
			
			//write new directory out to disk
			write_inode(new_inode);
		}
	} else {
		//not allowed, set err appropriately for not having correct permissions
		printf("mkdir: %s: Permission denied\n", absolute_path);
		err = -1;
	}
	
	return err;
}

// int ffs_readdir(const char *path, void *buf, off_t off, enum fuse_readdir_flags flags) {
//     struct dirent *d;
    
    
//     return 0;
// }


// int ffs_readdir(const char *path, void *buf,fuse_fill_dir_t filler, off_t off, struct fuse_file_info *fi, enum fuse_readdir_flags flags);
// {
	 
// 	// refer to http://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/html/unclear.html
// 	// find inode number of directory
// 	// read entries in directory, call filler function
// 	return 0;
// }

// int ffs_unlink(const char *path)
// {

// 	return 0;
// }

// int ffs_open(const char *path, struct fuse_file_info *fi)
// {

// 	return 0;
// }

// int ffs_read(const char *path, char *buf, size_t size, off_t off, struct fuse_file_info *fi)
// {

// 	return 0;
// }

// int ffs_write(const char *path, const char *buf, size_t size, off_t off, struct fuse_file_info *fi)
// {

// 	return 0;
// }

// int ffs_mknod(const char *path, mode_t mode, dev_t rdev)
// {

// 	return 0;
// }