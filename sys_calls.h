#ifndef SYS_CALLS_H
#define SYS_CALLS_H

#include "global.h"
#include "dir_alloc.h"

uint32_t ffs_mkdir(uint16_t mode, int p_flag, int v_flag, char* path, uint8_t uid[255], uint8_t gid[255]);
// int ffs_mknod(const char *path, mode_t mode, dev_t rdev);
// //int ffs_mkdir(const char *path, mode_t mode);
// int ffs_mkdir(uint16_t mode, int p_flag, int v_flag, char* path, char* pwd, uint8_t uid[255], uint8_t gid[255]);
// int ffs_readdir(const char *path, void *buf,fuse_fill_dir_t filler, off_t off, struct fuse_file_info *fi, enum fuse_readdir_flags flags);
// int ffs_unlink(const char *path);
// int ffs_open(const char *path, struct fuse_file_info *fi);
// int ffs_read(const char *path, char *buf, size_t size, off_t off, struct fuse_file_info *fi);
// int ffs_write(const char *path, const char *buf, size_t size, off_t off, struct fuse_file_info *fi);

#endif

/*

Files & Directories:

int ffs_mknod(const char *path, mode_t mode, dev_t rdev);
int ffs_mkdir(const char *path, mode_t mode);
int ffs_readdir(const char *path, void *buf,fuse_fill_dir_t filler, off_t off, struct fuse_file_info *fi, enum fuse_readdir_flags flags);
int ffs_unlink(const char *path);
int ffs_open(const char *path, struct fuse_file_info *fi);
int ffs_read(const char *path, char *buf, size_t size, off_t off, struct fuse_file_info *fi);
int ffs_write(const char *path, const char *buf, size_t size, off_t off, struct fuse_file_info *fi);

Additional Calls:

int ffs_getattr(const char *path, struct stat *buf, struct fuse_file_info *fi);
int ffs_rename(const char *oldpath, const char *newpath, unsigned int flags);
int ffs_rmdir(const char *path);
int ffs_symlink(const char *linkname, const char *path);
int ffs_link(const char *oldpath, const char *newpath);
int ffs_release(struct fuse_fs *fs,	 const char *path, struct fuse_file_info *fi);
int ffs_read_buf(const char *path, struct fuse_bufvec **bufp, size_t size, off_t off, struct fuse_file_info *fi);
int ffs_write_buf(const char *path, struct fuse_bufvec *buf, off_t off,struct fuse_file_info *fi);
int ffs_fsync(const char *path, int datasync, struct fuse_file_info *fi);
int ffs_flush(const char *path, struct fuse_file_info *fi);
int ffs_statfs(const char *path, struct statvfs *buf);
int ffs_opendir(const char *path, struct fuse_file_info *fi);
int ffs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi);
int ffs_releasedir(const char *path, struct fuse_file_info *fi);
int ffs_create(const char *path, mode_t mode, struct fuse_file_info *fi);
int ffs_lock(const char *path, struct fuse_file_info *fi, int cmd, struct flock *lock);
int ffs_flock(const char *path, struct fuse_file_info *fi, int op);
int ffs_chmod(const char *path, mode_t mode, struct fuse_file_info *fi);
int ffs_chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi);
int ffs_truncate(const char *path, off_t size, struct fuse_file_info *fi);
int ffs_utimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi);
int ffs_access(const char *path, int mask);
int ffs_readlink(const char *path, char *buf, size_t len);
int ffs_setxattr(const char *path, const char *name, const char *value, size_t size, int flags);
int ffs_getxattr(const char *path, const char *name, char *value, size_t size);
int ffs_listxattr(const char *path, char *list, size_t size);
int ffs_removexattr(const char *path, const char *name);
int ffs_bmap(const char *path, size_t blocksize, uint64_t *idx);
int ffs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi, unsigned int flags, void *data);
int ffs_poll(const char *path, struct fuse_file_info *fi, struct fuse_pollhandle *ph, unsigned *reventsp);
int ffs_fallocate(const char *path, int mode, off_t offset, off_t length, struct fuse_file_info *fi);
void ffs_init(struct fuse_conn_info *conn, struct fuse_config *cfg);
void ffs_destroy(struct fuse_fs *fs);

*/