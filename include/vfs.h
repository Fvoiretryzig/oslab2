#ifndef __VFS_H__
#define __VFS_H__

//#define O_RDONLY 0
//#define O_WRONLY 1
//#define O_RDWR 2
//#define O_CREATE 4

//#define SEEK_SET 0
//#define SEEK_CUR 1
//#define SEEK_END 2

#define file_content_maxn 1024
#define inode_cnt 512
#define name_len 128
#define fd_cnt 2048
#define file_cnt 2048


struct inode
{
	//int inode_no;
	int if_write;
	int if_read;
	int if_exist;	
	int thread_cnt;
	char name[name_len];
	char content[file_content_maxn];
	int size;	//文件大小
};
struct fileops 
{
	int (*open)(inode_t *inode, file_t *file, int flags);
	ssize_t (*read)(inode_t *inode, file_t *file, char *buf, size_t size);
	ssize_t (*write)(inode_t *inode, file_t *file, const char *buf, size_t size);
	off_t (*lseek)(inode_t *inode, file_t *file, off_t offset, int whence);
	int (*close)(inode_t *inode, file_t *file);
};
struct file
{
	int if_read;
	int if_write;
	inode_t* f_inode;
	int offset; //文件当前偏移量
	char content[file_content_maxn];
	char name[name_len];
	int fd;
	fileops_t *ops;
};
struct fsops 
{
	void (*init)(struct filesystem *fs, const char *name, inode_t *dev);
	inode_t *(*lookup)(struct filesystem *fs, const char *path, int flags);
	int (*close)(inode_t *inode);
};
struct filesystem
{
	char name[name_len];
	mountpath_t* path;
	inode_t *inode[inode_cnt];
	fsops_t *ops;
};
struct mount_path
{
	char p[64];
	filesystem_t* fs;
};
#endif

