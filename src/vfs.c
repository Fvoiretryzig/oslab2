#include <os.h>
#include <vfs.h>
#include<libc.h>


//typedef struct filesystem filesystem_t;
//typedef struct inode inode_t;
//typedef struct file file_t;
static void vfs_init();
static int access(const char *path, int mode);
static int mount(const char *path, filesystem_t *fs);
static int unmount(const char *path);
static int open(const char *path, int flags);
static ssize_t read(int fd, void *buf, size_t nbyte);
static ssize_t write(int fd, void *buf, size_t nbyte);
static off_t lseek(int fd, off_t offset, int whence);
static int close(int fd);

static int inode_num_proc;// = 0;
static int inode_num_dev;
static int inode_num_kv;
mountpath_t* procfs_p;
mountpath_t* devfs_p;
extern mountpath_t* kvfs_p;
mountpath_t* kvfs_p;
int fd[fd_cnt];
file_t* file_table[file_cnt];

int seed;

spinlock_t vfs_lk;
MOD_DEF(vfs)
{
	.init = vfs_init,
	.access = access,
	.mount = mount,
	.unmount = unmount,
	.open = open,
	.read = read,
	.write = write,
	.lseek = lseek,
	.close = close,
};

  /*====================================================================*/
 /*==============================vfs init==============================*/
/*====================================================================*/
static fsops_t *procfs_op;
static fsops_t *devfs_op;
static fsops_t *kvfs_op;
void procfs_init(filesystem_t *fs, inode_t *dev)
{
	/*================cpuinfo================*/
	if(inode_num_proc == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}
	inode_t *cpuinfo = (inode_t *)pmm->alloc(sizeof(inode_t));
	cpuinfo->if_exist = 1;
	cpuinfo->if_write = 0; cpuinfo->if_read = 1;
	strcpy(cpuinfo->name, "/proc");
	strcat(cpuinfo->name, "/cpuinfo");
	char* c_info = "My cpuinfo:remain to be done";
	strcpy(cpuinfo->content, c_info);
	cpuinfo->size = strlen(c_info);
	fs->inode[inode_num_proc++] = cpuinfo;
	pmm->free(cpuinfo);
	/*================meminfo================*/
	if(inode_num_proc == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}		
	inode_t *meminfo = (inode_t *)pmm->alloc(sizeof(inode_t));
	//meminfo->inode_no = inode_num_proc; 
	meminfo->if_exist = 1;
	meminfo->if_write = 0; meminfo->if_read = 1;
	strcpy(meminfo->name, "/proc");
	strcat(meminfo->name, "/meminfo");
	char* m_info = "My meminfo:remain to be done";
	strcpy(meminfo->content, m_info);
	meminfo->size = strlen(m_info);
	fs->inode[inode_num_proc++] = meminfo;	
	pmm->free(meminfo);
	/*for(int i = inode_num_proc; i<inode_cnt; i++){
		fs->inode[i]->if_exist = 0;
		fs->inode[i]->if_read = 0;
		fs->inode[i]->if_write = 0;
		fs->inode[i]->thread_cnt = 0;
	}*/		
	return;
}
void devfs_init(filesystem_t *fs, inode_t *dev)
{
	/*================null================*/	
	if(inode_num_dev == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}
	inode_t *null = (inode_t *)pmm->alloc(sizeof(inode_t));
	//null->inode_no = inode_num_dev; 
	null->if_exist = 1;
	null->if_write = 1; null->if_read = 1; null->size = 0;
	strcpy(null->name, "/dev");
	strcat(null->name, "/null");
	fs->inode[inode_num_dev++] = null;
	pmm->free(null);
	/*================zero================*/	
	if(inode_num_dev == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}	
	inode_t *zero = (inode_t *)pmm->alloc(sizeof(inode_t));
	//zero->inode_no = inode_num_dev; 
	zero->if_exist = 1;
	zero->if_write = 1; zero->if_read = 1; zero->size = 0;
	strcpy(zero->name, "/dev");
	strcat(zero->name, "/zero");
	fs->inode[inode_num_dev++] = zero;
	pmm->free(zero);
	/*================random================*/		
	if(inode_num_dev == inode_cnt){
		printf("the inode is full in procfs\n");
		return;
	}	
	inode_t *random = (inode_t *)pmm->alloc(sizeof(inode_t));
	//random->inode_no = inode_num_dev;
	random->if_exist = 1; 
	random->if_write = 1; random->if_read = 1; random->size = 0;	//不知道要初始成多大
	strcpy(random->name,"/dev");
	strcat(random->name, "/random");
	fs->inode[inode_num_dev++] = random;
	pmm->free(random);
	/*for(int i = inode_num_dev; i<inode_cnt; i++){
		fs->inode[i]->if_exist = 0;
		fs->inode[i]->if_read = 0;
		fs->inode[i]->if_write = 0;
		fs->inode[i]->thread_cnt = 0;
	}*/
	return;		
}
void kvfs_init(filesystem_t *fs, inode_t *dev)
{
	//TODO();目前不知道这里kvfs如何初始化
	/*for(int i = 0; i<inode_cnt ; i++){
		fs->inode[i]->if_exist = 0;
		fs->inode[i]->if_read = 0;
		fs->inode[i]->if_write = 0;
		fs->inode[i]->thread_cnt = 0;
	}*/
	return;
}
void fs_init(filesystem_t *fs, const char *name, inode_t *dev)	//dev的作用
{
	for(int i = 0; i<inode_cnt; i++){
		fs->inode[i] = NULL;
	}
	if(!strcmp(name, "procfs")){
		procfs_init(fs, dev);		
	}
	else if(!strcmp(name, "devfs")){
		devfs_init(fs, dev);
	}
	else if(!strcmp(name, "kvfs")){
		kvfs_init(fs, dev);
	}
}
inode_t *lookup(filesystem_t *fs, const char *path, int flag)
{	
	//printf("lookup:path:%s\n", path);
	inode_t *ans = NULL;	//????????????????
	int index = 0; int if_find = 0;
	while(index < inode_cnt){
		//printf("lookup: inode[%d]:name:%s if_exist:%d\n",index, fs->inode[index]->name,fs->inode[index]->if_exist );
		if(fs->inode[index]->if_exist){
			//printf("lookup: inode[%d]:name:%s\n",index, fs->inode[index]->name);
			if(!strcmp(path, fs->inode[index]->name)){
				//printf("lookup:inode[%d] if_read:%d if_write:%d\n", index, fs->inode[index]->if_read, fs->inode[index]->if_write);
				if_find = 1;
				break;
			}			
		}
		index++;
	}
	if(if_find && index < inode_cnt){
		ans = fs->inode[index];	
	}
	//printf("lookup:if_find:%d\n", if_find);
	return ans;
}
int fs_close(inode_t *inode)
{
	//TODO()目前不知道这里要干什么
	return 0;
}
void fsop_init()
{
	procfs_op = (fsops_t*)pmm->alloc(sizeof(fsops_t));
	procfs_op->init = &fs_init;
	procfs_op->lookup = &lookup;
	procfs_op->close = &fs_close;
	
	devfs_op = (fsops_t*)pmm->alloc(sizeof(fsops_t));
	devfs_op->init = &fs_init;
	devfs_op->lookup = &lookup;
	devfs_op->close = &fs_close;	
	
	kvfs_op = (fsops_t*)pmm->alloc(sizeof(fsops_t));
	kvfs_op->init = &fs_init;
	kvfs_op->lookup = &lookup;
	kvfs_op->close = &fs_close;		
	
	return;		
}
filesystem_t *create_procfs() 
{
	filesystem_t *fs = (filesystem_t *)pmm->alloc(sizeof(filesystem_t));
	strcpy(fs->name, "procfs");
	if (!fs) panic("procfs allocation failed");
	fs->ops = procfs_op; // 你为procfs定义的fsops_t，包含函数的实现
	fs->ops->init(fs, "procfs", NULL);
	
	return fs;
}
filesystem_t *create_devfs() 
{
	filesystem_t *fs = (filesystem_t *)pmm->alloc(sizeof(filesystem_t));
	strcpy(fs->name, "devfs"); 
	if (!fs) panic("devfs allocation failed");
	fs->ops = devfs_op; // 你为procfs定义的fsops_t，包含函数的实现
	fs->ops->init(fs, "devfs", NULL);
	
	return fs;
}
filesystem_t *create_kvfs() 
{
	filesystem_t *fs = (filesystem_t *)pmm->alloc(sizeof(filesystem_t));
	strcpy(fs->name, "kvfs"); 
	if (!fs) panic("fs allocation failed");
	fs->ops = kvfs_op; // 你为procfs定义的fsops_t，包含函数的实现
	fs->ops->init(fs, "kvfs", NULL);
	return fs;
}
int mount(const char *path, filesystem_t *fs)
{
	if(!strcmp(path, "/proc")){
		strcpy(procfs_p->p, path);
		procfs_p->fs = fs;
		fs->path = procfs_p;
	}
	else if(!strcmp(path, "/dev")){
		strcpy(devfs_p->p, path);
		devfs_p->fs = fs;
		fs->path = devfs_p;
	}
	else if(!strcmp(path, "/")){
		strcpy(kvfs_p->p, path);
		kvfs_p->fs = fs;
		fs->path = kvfs_p;
		
	}
	else{
		printf("wrong when mount %s!!!!\n", path);
		return -1;
	}
	return 0;
}
int unmount(const char *path)
{
	//TODO!!!!!!!???????
	if(!strcmp(path, "/proc")){
		procfs_p->fs->path = NULL;
	}
	else if(!strcmp(path, "/dev")){
		devfs_p->fs->path = NULL;
	}
	else if(!strcmp(path, "/")){
		kvfs_p->fs->path = NULL;
	}
	else{
		printf("error when unmount %s\n", path);
	}
	return 0;
}
  /*====================================================================*/
 /*==============================file ops==============================*/
/*====================================================================*/
fileops_t *procfile_op;
fileops_t *devfile_op;
fileops_t *kvfile_op;
int file_open(inode_t *inode, file_t *file, int flags)
{
	int current_fd = -1;
	switch(flags){
		case O_RDONLY:
			if(!inode->if_exist){
				printf("cannot open the file which is not existing while reading!\n");
				return -1;
			}
			else if(!inode->if_read){
				printf("open O_RDONLY can not read in file_open:%s\n", inode->name);
				printf("open mode error: have no permission to read %s\n", inode->name);
				return -1;
			}
			for(int i = 3; i<fd_cnt; i++){
				if(fd[i] == 0){
					fd[i] = 1;
					current_fd = i;
					break;
				}
			}
			if(current_fd == -1){
				printf("open fd error: there isn't enough fd left in read!\n");
				return -1;
			}
			file->fd = current_fd;
			strcpy(file->name, inode->name);
			strcpy(file->content, inode->content);
			file->f_inode = inode;
			file->offset = 0;	
			file->if_read = 1;
			file->if_write = 0;									
			file_table[current_fd] = file;
			break;
		case O_WRONLY:
			if(!inode->if_exist){
				printf("cannot open the file which is not existing while writing!\n");
				return -1;
			}
			else if(!inode->if_write){
				printf("open mode error: have no permission to write%s\n", inode->name);
				return -1;
			}
			for(int i = 3; i<fd_cnt; i++){
				if(fd[i] == 0){
					fd[i] = 1;
					current_fd = i;
					break;
				}
			}
			if(current_fd == -1){
				printf("open fd error: there isn't enough fd left in write!\n");
				return -1;
			}
			file->fd = current_fd;
			strcpy(file->name, inode->name);
			strcpy(file->content, inode->content);
			file->f_inode = inode;
			file->offset = 0;	
			file->if_read = 0;
			file->if_write = 1;									
			file_table[current_fd] = file;
			break;
		case O_RDWR:
			if(!inode->if_exist){			
				printf("cannot open the file which is not existing while writing and reading!\n");
				return -1;
			}
			else if(!inode->if_write || !inode->if_read){
				printf("open mode error: have no permission to write or read %s\n", inode->name);
				return -1;
			}
			for(int i = 3; i<fd_cnt; i++){
				if(fd[i] == 0){
					fd[i] = 1;
					current_fd = i;
					break;
				}
			}
			if(current_fd == -1){
				printf("open fd error: there isn't enough fd left in read&write!\n");
				return -1;
			}
			//printf("O_WRDR:kvfs_p->fs->inode[0]:%s if_read:%d if_write:%d\n", kvfs_p->fs->inode[0]->name,kvfs_p->fs->inode[0]->if_read, kvfs_p->fs->inode[0]->if_write);
			file->fd = current_fd;
			strcpy(file->name, inode->name);
			strcpy(file->content, inode->content);

			file->f_inode = inode;
			file->offset = 0;	
			file->if_read = 1;
			file->if_write = 1;									
			file_table[current_fd] = file;	
			break;	
		case O_CREATE:
			if(inode->if_exist){
				printf("this file %s has already existed!", inode->name);
				return -1;
			}
			for(int i = 3; i<fd_cnt; i++){
				if(fd[i] == 0){
					fd[i] = 1;
					current_fd = i;
					break;
				}
			}	
			if(current_fd == -1){
				printf("open fd error: there isn't enough fd left in create!\n");
				return -1;
			}
			inode->if_exist = 1; inode->if_read = 1; inode->if_write = 0;
			inode->thread_cnt++;
			inode->size = 0; //inode->content = "";
			strcpy(inode->content, "");
			
			file->fd = current_fd;
			strcpy(file->name, inode->name);
			strcpy(file->content, inode->content);
			file->f_inode = inode;
			file->offset = 0;	
			file->if_read = 1;
			file->if_write = 0;									
			file_table[current_fd] = file;
			break;									
		case O_CREATE|O_WRONLY:
			if(inode->if_exist){
				printf("this file %s has already existed!", inode->name);
				return -1;
			}
			for(int i = 3; i<fd_cnt; i++){
				if(fd[i] == 0){
					fd[i] = 1;
					current_fd = i;
					break;
				}
			}	
			if(current_fd == -1){
				printf("open fd error: there isn't enough fd left in create!\n");
				return -1;
			}
			inode->if_exist = 1; inode->if_read = 0; inode->if_write = 1;
			inode->size = 0; //inode->content = "";
			strcpy(inode->content, "");
			
			file->fd = current_fd;
			strcpy(file->name, inode->name);
			strcpy(file->content, inode->content);
			file->f_inode = inode;
			file->offset = 0;	
			file->if_read = 0;
			file->if_write = 1;									
			file_table[current_fd] = file;
			break;				
		case O_CREATE|O_RDWR:
			if(inode->if_exist){
				printf("this file %s has already existed!", inode->name);
				return -1;
			}
			for(int i = 3; i<fd_cnt; i++){
				if(fd[i] == 0){
					fd[i] = 1;
					current_fd = i;
					break;
				}
			}	
			if(current_fd == -1){
				printf("open fd error: there isn't enough fd left in create!\n");
				return -1;
			}
			inode->if_exist = 1; inode->if_read = 1; inode->if_write = 1;
			inode->size = 0; //inode->content[0] = '\0';
			strcpy(inode->content, "");
			printf("file_open:inode->name:%s\n", inode->name);			
			
			file->fd = current_fd;
			strcpy(file->name, inode->name);
			strcpy(file->content, inode->content);
			file->f_inode = inode;
			file->offset = 0;	
			file->if_read = 1;
			file->if_write = 1;									
			file_table[current_fd] = file;
			//printf("file_open:current_fd:%d file->offset:%d",current_fd, file->offset);
			break;				
	}
	return file->fd;
}
ssize_t kvproc_file_read(inode_t *inode, file_t *file, char *buf, size_t size)
{
	//printf("in file_read:name:%s size:%d inode_size:%d offset:%d\n", file->name, size, inode->size,file->offset);
	if(!inode->if_read){
		printf("read permission error: cannot read %s\n", file->name);
		return -1;
	}
	if(size > file->f_inode->size - file->offset){
		size = inode->size - file->offset;
	}
	strncpy(buf, file->content+file->offset, size);
	file->offset += size;
	return size;
}
ssize_t dev_file_read(inode_t *inode, file_t *file, char*buf, size_t size)
{
	if(!inode->if_read){
		printf("read permission error: cannot read %s\n", file->name);
		return -1;
	}
	if(size > file->f_inode->size - file->offset){
		size = inode->size - file->offset;
	}	
	if(!strcmp(inode->name+strlen(devfs_p->p), "/zero")){
		strcpy(buf, "");
	}
	else if(!strcmp(inode->name+strlen(devfs_p->p), "/null")){
		strcpy(buf, "");
	}
	else if(!strcmp(inode->name+strlen(devfs_p->p), "/random")){	
		int num = rand() % 8192;
		strncpy(buf, itoa(num), 4);
	}
	else{
		strncpy(buf, file->content+file->offset, size);
	}
	file->offset += size;
	return size;
}
ssize_t kvproc_file_write(inode_t *inode, file_t *file, const char *buf, size_t size)
{
	//printf("in file_write:name:%s buf:%s size:%d\n", inode->name, buf, size);
	if(!inode->if_write){
		printf("write permission error: cannot write %s\n", file->name);
		return -1;
	}
	//printf("file_write:before renew size:%d\n", size);
	if((file->offset + size) >= file_content_maxn){
		size = file_content_maxn - file->offset;
	}
	strncpy(inode->content + file->offset, buf, size);
	strcpy(file->content, inode->content);	//先拷贝到inode再到文件
	inode->size = file->offset + size;
	file->offset += size;
	//printf("file_write:size:%d\n", size);
	return size;
}
ssize_t dev_file_write(inode_t *inode, file_t *file, const char *buf, size_t size)
{
	printf("in file_write:name:%s buf:%s size:%d\n", inode->name, buf, size);
	if(!inode->if_write){
		printf("write permmison error: cannot write %s\n", file->name);
		return -1;
	}
	if(!strcmp(inode->name+strlen(devfs_p->p), "/zero")
	|| !strcmp(inode->name+strlen(devfs_p->p), "/null")
	|| !strcmp(inode->name+strlen(devfs_p->p), "/random")){
		return size;	//这几个文件写了也没用
	}
	else if(file->offset + size >= file_content_maxn){
		size = file_content_maxn - file->offset;
	}
	strncpy(inode->content + file->offset, buf, size);
	strcpy(file->content, inode->content);	//先拷贝到inode再到文件	
	inode->size = file->offset + size;
	file->offset += size;
	return size;
}
off_t file_lseek(inode_t *inode, file_t *file, off_t offset, int whence)
{
	switch(whence){
		case SEEK_SET:
			if(offset >= file_content_maxn){
				printf("cannot set offset larger than file_content_maxn in SEEK_SET %s\n", file->name);
				return -1;
			}
			file->offset = offset;
			break;
		case SEEK_CUR:
			if((file->offset + offset) >= file_content_maxn){
				printf("cannot set offset larger than file_content_maxn in SEEK_CUR %s\n", file->name);
				return -1;
			}
			file->offset += offset;
			break;
		case SEEK_END:
			if((inode->size + offset) >= file_content_maxn){
				printf("cannot set offset larger than file_content_maxn in SEEK_END %s\n", file->name);
				return -1;
			}
			file->offset = inode->size + offset;
			break;
	}
	return file->offset;
}
int file_close(inode_t *inode, file_t *file)
{
	int current_fd = file->fd;
	printf("in file_close:fd:%d\n", current_fd);
	fd[current_fd] = 0;
	file_table[current_fd] = NULL;
	return 0;	//不知道什么时候会是-1
}
void fileop_init()
{
	procfile_op = (fileops_t*)pmm->alloc(sizeof(fileops_t));
	procfile_op->open = &file_open;
	procfile_op->read = &kvproc_file_read;
	procfile_op->write = &kvproc_file_write;
	procfile_op->lseek = &file_lseek;
	procfile_op->close = &file_close;
	
	devfile_op = (fileops_t*)pmm->alloc(sizeof(fileops_t));
	devfile_op->open = &file_open;
	devfile_op->read = &dev_file_read;
	devfile_op->write = &dev_file_write;
	devfile_op->lseek = &file_lseek;
	devfile_op->close = &file_close;
	
	kvfile_op = (fileops_t*)pmm->alloc(sizeof(fileops_t));
	kvfile_op->open = &file_open;
	kvfile_op->read = &kvproc_file_read;
	kvfile_op->write = &kvproc_file_write;
	kvfile_op->lseek = &file_lseek;
	kvfile_op->close = &file_close;	
	
	return;		
}
void vfs_init()
{
	
	fd[0] = 1; fd[1] = 1; fd[2] = 1;
	for(int i = 3; i<fd_cnt; i++){
		fd[i] = 0;
	}
	for(int i = 0; i<file_cnt; i++){
		file_table[i] = NULL;		
	}
	/*========random seed init========*/
	seed = 40;
	srand(seed);
	/*================================*/
	inode_num_proc = 0;
	inode_num_dev = 0;
	inode_num_kv = 0;
	fsop_init();
	fileop_init();
	procfs_p = pmm->alloc(sizeof(mountpath_t));
	devfs_p = pmm->alloc(sizeof(mountpath_t));
	kvfs_p = pmm->alloc(sizeof(mountpath_t));
	mount("/proc", create_procfs());
	mount("/dev", create_devfs());
	mount("/", create_kvfs());
	for(int i = 2; i<inode_cnt; i++){
		procfs_p->fs->inode[i]->if_exist = 0;
	}
	kmt->spin_init(&vfs_lk, "vfs_lk");
	return;
}
  /*====================================================================*/
 /*==============================sys_call==============================*/
/*====================================================================*/
int access(const char *path, int mode)
{
	int ret = 0;
	kmt->spin_lock(&vfs_lk);

	/*=========================lock=========================*/
	inode_t *temp = pmm->alloc(sizeof(inode_t));
	if(!strncmp(path, procfs_p->p, strlen(procfs_p->p))){
		temp = procfs_p->fs->ops->lookup(procfs_p->fs, path, mode);	//不知道是不是mode
	}
	else if(!strncmp(path, devfs_p->p, strlen(devfs_p->p))){
		temp = devfs_p->fs->ops->lookup(devfs_p->fs, path, mode);
	}
	else if(!strncmp(path, kvfs_p->p, strlen(kvfs_p->p))){
		temp = kvfs_p->fs->ops->lookup(kvfs_p->fs, path, mode);
	}
	switch(mode){
		case F_OK:
			if(temp == NULL){
				printf("the file has not been created!!\n");
				ret = -1;
			}
			else{
				printf("access:temp name:%s\n", temp->name);
			}
			break;
		case X_OK:
		case X_OK|W_OK:
		case X_OK|R_OK:
			printf("remain to be done to support executable file\n");
			break;
		case W_OK:
			if(temp == NULL){
				printf("the file has not been created!!\n");
				ret = -1;
			}
			else if(!temp->if_write){
				printf("have no permission to write when check in access %s\n", path);
				ret = -1;
			}
			break;
		case R_OK:
			if(temp == NULL){
				printf("the file has not been created!!\n");
				ret = -1;
			}		
			else if(!temp->if_read){
				printf("have no permission to read when check in access %s\n", path);
				ret = -1;
			}
			break;
		case W_OK|R_OK:
			if(temp == NULL){
				printf("the file has not been created!!\n");
				ret = -1;
			}		
			else if(!temp->if_read || !temp->if_write){
				printf("have no permission to read or write when check in access %s\n", path);
				ret = -1;
			}
			break;
	}
	pmm->free(temp);
	/*=========================unlock=========================*/
	kmt->spin_unlock(&vfs_lk);
	return ret;
}

int open(const char *path, int flags)
{
	kmt->spin_lock(&vfs_lk);
	//printf("procfs_p->fs->inode[0]->name:%s if_exist:%d\n",procfs_p->fs->inode[0]->name,procfs_p->fs->inode[0]->if_exist);
	//printf("OPEN:procfs_p->fs->inode[0]:%s if_read:%d if_write:%d\n", procfs_p->fs->inode[0]->name,procfs_p->fs->inode[0]->if_read, procfs_p->fs->inode[0]->if_write);
	/*=========================lock=========================*/
	inode_t* node = NULL; 
	file_t *FILE = (file_t*)pmm->alloc(sizeof(file_t));
	//file_t FILE;
	//printf("open: FILE address:0x%08x inode address:0x%08x\n", &FILE, kvfs_p->fs->inode[0]); 
	FILE->if_read = 0; FILE->if_write = 0;
//printf("O_WRDR:kvfs_p->fs->inode[0]:%s if_read:%d if_write:%d\n", kvfs_p->fs->inode[0]->name,kvfs_p->fs->inode[0]->if_read, kvfs_p->fs->inode[0]->if_write);	//有bug这里的if_read和if_write被修改了！	
	if(!strncmp(path, procfs_p->p, strlen(procfs_p->p))){
		node = procfs_p->fs->ops->lookup(procfs_p->fs, path, flags);	//不知道是不是flag
		FILE->ops = procfile_op;
		if(node == NULL){
			if(inode_num_proc == inode_cnt){
				printf("the file is not exisiting while open and there is no inode to allocate!\n");
				return -1;
			}
			node = pmm->alloc(sizeof(inode_t));
			node->if_exist = 0; node->if_read = 0; node->if_write = 0; node->thread_cnt = 0; node->size = 0;
			procfs_p->fs->inode[inode_num_proc++] = node;
			strcpy(node->name, path);
		}
		else{
	/*=========================unlock=========================*/
			kmt->spin_unlock(&vfs_lk);	
			while(node->thread_cnt > 0);
			kmt->spin_lock(&vfs_lk);
	/*=========================lock=========================*/						
		}
	}
	else if(!strncmp(path, devfs_p->p, strlen(devfs_p->p))){
		node = devfs_p->fs->ops->lookup(devfs_p->fs, path, flags);
		FILE->ops = devfile_op;
		if(node == NULL){
			if(inode_num_dev == inode_cnt){
				printf("the file is not exisiting while open and there is no inode to allocate!\n");
				return -1;
			}
			node = pmm->alloc(sizeof(inode_t));
			node->if_exist = 0; node->if_read = 0; node->if_write = 0; node->thread_cnt = 0; node->size = 0;			
			devfs_p->fs->inode[inode_num_dev++] = node;
			strcpy(node->name, path);
		}
		else{
	/*=========================unlock=========================*/
			kmt->spin_unlock(&vfs_lk);	
			while(node->thread_cnt > 0);
			kmt->spin_lock(&vfs_lk);
	/*=========================lock=========================*/						
		}				
	}
	else if(!strncmp(path, kvfs_p->p, strlen(kvfs_p->p))){	
		node = kvfs_p->fs->ops->lookup(kvfs_p->fs, path, flags);
		FILE->ops = kvfile_op;
		if(node == NULL){
			if(inode_num_kv == inode_cnt){
				printf("the file is not exisiting while open and there is no inode to allocate!\n");
				return -1;
			}
			node = pmm->alloc(sizeof(inode_t));
			node->if_exist = 0; node->if_read = 0; node->if_write = 0; node->thread_cnt = 0; node->size = 0;
			strcpy(node->name, path);
			kvfs_p->fs->inode[inode_num_kv++] = node;
			//printf("open:kvfs_p->fs->inode[%d]:%s\n",inode_num_kv-1, kvfs_p->fs->inode[inode_num_kv-1]->name);
		}	
		else{
	/*=========================unlock=========================*/
			kmt->spin_unlock(&vfs_lk);
			//printf("open:node name:%s if_read:%d if_write:%d\n", node->name, node->if_read, node->if_write);	
			while(node->thread_cnt > 0);
			kmt->spin_lock(&vfs_lk);
	/*=========================lock=========================*/						
		}			
	}
	int temp_fd = FILE->ops->open(node, FILE, flags);
	//pmm->free(FILE); 
	//printf("kuaidianxiehaoba!!\n");
	pmm->free(node);
	//printf("open:FILE->offset:%d\n", FILE->offset);
	/*=========================unlock=========================*/
	kmt->spin_unlock(&vfs_lk);	
	return temp_fd;
}
ssize_t read(int fd, void *buf, size_t nbyte)
{
	kmt->spin_lock(&vfs_lk);
	/*=========================lock=========================*/
	if(fd < 0){
		printf("invalid fd:%d in read\n", fd);
		return -1;
	}
	inode_t* node = NULL; 
	file_t *FILE = file_table[fd];	//还未实现描述符为0、1、2的操作
	char *path = FILE->name;
	if(!strncmp(path, procfs_p->p, strlen(procfs_p->p))){
		node = procfs_p->fs->ops->lookup(procfs_p->fs, path, 0);
	}
	else if(!strncmp(path, devfs_p->p, strlen(devfs_p->p))){
		node = devfs_p->fs->ops->lookup(devfs_p->fs, path, 0);
	}
	else if(!strncmp(path, kvfs_p->p, strlen(kvfs_p->p))){
		node = kvfs_p->fs->ops->lookup(kvfs_p->fs, path, 0);
	}
	if(node == NULL){
		printf("invalid read for a non-exiting inode!\n");
		return -1;
	}	
	ssize_t size = FILE->ops->read(node, FILE, buf, nbyte);
	/*=========================unlock=========================*/
	kmt->spin_unlock(&vfs_lk);	
	return size;
}
ssize_t write(int fd, void *buf, size_t nbyte)
{
	kmt->spin_lock(&vfs_lk);
	/*=========================lock=========================*/
	
	if(fd < 0){
		printf("invalid fd:%d in read\n", fd);
		return -1;
	}
	inode_t* node = NULL;
	file_t *FILE = file_table[fd];
	char *path = FILE->name;
	if(!strncmp(path, procfs_p->p, strlen(procfs_p->p))){
		node = procfs_p->fs->ops->lookup(procfs_p->fs, path, 0);
	}
	else if(!strncmp(path, devfs_p->p, strlen(devfs_p->p))){
		node = devfs_p->fs->ops->lookup(devfs_p->fs, path, 0);
	}
	else if(!strncmp(path, kvfs_p->p, strlen(kvfs_p->p))){
		
		node = kvfs_p->fs->ops->lookup(kvfs_p->fs, path, 0);
	}
	if(node == NULL){
		printf("invalid write for a non-exising inode!\n");
		return -1;
	}	
	//printf("write:in before file_write size:%d\n", nbyte);
	ssize_t size = FILE->ops->write(node, FILE, buf, nbyte);
	//printf("write:in after file_write size:%d\n", size);
	/*=========================unlock=========================*/
	kmt->spin_unlock(&vfs_lk);		
	return size;
}
off_t lseek(int fd, off_t offset, int whence)
{
	kmt->spin_lock(&vfs_lk);
	/*=========================lock=========================*/
	if(fd < 0){
		printf("invalid fd:%d in read\n", fd);
		return -1;
	}
	inode_t* node = NULL;
	file_t *FILE = file_table[fd];
	char *path = FILE->name;
	if(!strncmp(path, procfs_p->p, strlen(procfs_p->p))){
		node = procfs_p->fs->ops->lookup(procfs_p->fs, path, 0);
	}
	else if(!strncmp(path, devfs_p->p, strlen(devfs_p->p))){
		node = devfs_p->fs->ops->lookup(devfs_p->fs, path, 0);
	}
	else if(!strncmp(path, kvfs_p->p, strlen(kvfs_p->p))){
		node = kvfs_p->fs->ops->lookup(kvfs_p->fs, path, 0);
	}
	if(node == NULL){
		printf("invalid lseek for a non-existing inode!\n");
		return -1;
	}	
	off_t temp_offset = FILE->ops->lseek(node, FILE, offset, whence);	
	/*=========================unlock=========================*/
	kmt->spin_unlock(&vfs_lk);		
	return temp_offset;
}
int close(int fd)
{
	kmt->spin_lock(&vfs_lk);
	/*=========================lock=========================*/
	if(fd < 0){
		printf("invalid fd:%d in read\n", fd);
		return -1;
	}
	inode_t* node = NULL;
	file_t *FILE = file_table[fd];
	char *path = FILE->name;
	if(!strncmp(path, procfs_p->p, strlen(procfs_p->p))){
		node = procfs_p->fs->ops->lookup(procfs_p->fs, path, 0);
	}
	else if(!strncmp(path, devfs_p->p, strlen(devfs_p->p))){
		node = devfs_p->fs->ops->lookup(devfs_p->fs, path, 0);
	}
	else if(!strncmp(path, kvfs_p->p, strlen(kvfs_p->p))){
		node = kvfs_p->fs->ops->lookup(kvfs_p->fs, path, 0);
	}
	if(node == NULL){
		printf("invalid close for a non-existing inode!\n");
	}	
	int ret = FILE->ops->close(node, FILE);	
	node->thread_cnt--;	//不知道放锁里面还是外面
	/*=========================unlock=========================*/
	kmt->spin_unlock(&vfs_lk);		
	return ret;
}

