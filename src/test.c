#include<test.h>

  /*=======================================================*/
 /*====================test for thread====================*/
/*=======================================================*/
extern spinlock_t lk;
spinlock_t lk;
spinlock_t lk_thread;
void producer()
{
	while (1) {
		kmt->sem_wait(&empty);
		printf("(");
		kmt->sem_signal(&fill);
	}
}
void consumer()
{
	while (1) {
		kmt->sem_wait(&fill);
		printf(")");
		kmt->sem_signal(&empty);
	}
}
void test_run()
{
	kmt->spin_init(&lk,"intestrun");
	kmt->spin_lock(&lk);
	kmt->sem_init(&empty, "empty", BUF_SIZE);
	kmt->sem_init(&fill, "fill", 0);
	
  	kmt->create(&t1, &producer, NULL);
  	kmt->create(&t2, &consumer, NULL);
  	kmt->create(&t3, &producer, NULL);
  	kmt->create(&t4, &consumer, NULL);	
  	kmt->create(&t5, &producer, NULL);
  	kmt->create(&t6, &consumer, NULL);
  	kmt->create(&t7, &consumer, NULL);
  	kmt->create(&t8, &producer, NULL);
  	kmt->create(&t9, &consumer, NULL);
  	kmt->create(&t10, &consumer, NULL);
	kmt->create(&t11, &producer, NULL);
  	kmt->create(&t12, &consumer, NULL);
  	kmt->create(&t13, &consumer, NULL);
  	kmt->create(&t14, &producer, NULL);
  	kmt->create(&t15, &consumer, NULL);
  	kmt->create(&t16, &consumer, NULL);
  	kmt->spin_unlock(&lk);
}
  /*=======================================================*/
 /*=====================test for file=====================*/
/*=======================================================*/
void dev_test()
{
	while(1){
		//kmt->spin_lock(&lk);
		char buf[1024]; int size = 0;	
		/*========================random========================*/
		int random_fd = vfs->open("/dev/random", O_RDONLY);
		printf("dev_test:random fd:%d\n", random_fd);
		size = vfs->read(random_fd, buf, 0);
		if(size < 0){
			printf("dev_test:error read /dev/random in dev_test\n");
			vfs->close(random_fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		printf("dev_test:this is the random number return by /dev/random:%s size:%d\n", buf, size);
		size = vfs->read(random_fd, buf, 0);
			if(size < 0){
			printf("dev_test:error read /dev/random in dev_test\n");
			vfs->close(random_fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		printf("dev_test:this is the random number return by /dev/random:%s size:%d\n", buf, size);
		/*========================null========================*/
		int null_fd = vfs->open("/dev/null", O_RDWR);
			printf("dev_test:null fd:%d\n", null_fd);
		strcpy(buf, "40404040");
		size = vfs->write(null_fd, buf, strlen(buf));
		if(size < 0){
			printf("dev_test:error write /dev/null\n");
			vfs->close(null_fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		printf("dev_test:this is the writing /dev/null operation return value:%d\n", size);
		size = vfs->read(null_fd, buf, 0);
		if(size < 0){
			printf("dev_test:error read /dev/null\n");
			vfs->close(null_fd);;
			kmt->spin_unlock(&lk);
			_yield();
		}
		printf("dev_test:after read /dev/null buf:%d\n", *buf);
		vfs->close(null_fd);
		/*========================zero========================*/
		int zero_fd = vfs->open("/dev/zero", O_RDONLY);
		printf("dev_test:zero fd:%d\n", zero_fd);
		strcpy(buf, "40404040");
		printf("dev_test:this is buf in zero before read buf:%s\n", buf);
		size = vfs->read(zero_fd, buf, 0);
		if(size < 0){
			printf("dev_test:error read /dev/zero\n");
			vfs->close(zero_fd);
			kmt->spin_unlock(&lk);
			_yield();
		}	
		printf("dev_test:after read /dev/zero buf:%d\n", *buf);
		vfs->close(zero_fd);
		vfs->close(random_fd);
		printf("dev_test:end\n\n");
		//kmt->spin_unlock(&lk);
		//_yield();
	}		
	return;
}
void proc_test()
{
	while(1){
		//kmt->spin_lock(&lk);
		char buf[1024]; int size = 0;
	/*========================cpuinfo========================*/
		int cpu_fd = vfs->open("/proc/cpuinfo", O_RDONLY);
		printf("proc_test: the cpu fd is %d\n", cpu_fd);
		if(cpu_fd < 0){
			printf("proc_test:open cpuinfo error!\n");
			kmt->spin_unlock(&lk);
			_yield();
		}
		
		size = vfs->read(cpu_fd, buf, 1024);
		if(size < 0){
			printf("proc_test:read error while read cpuinfo\n");
			vfs->close(cpu_fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		printf("proc_test:size:%d\ncontent:\n%s\n", size, buf);
		vfs->close(cpu_fd);
	/*========================meminfo========================*/
		int mem_fd = vfs->open("/proc/meminfo", O_RDONLY);
		if(mem_fd < 0){
			printf("proc_test:open meminfo error!\n");
			kmt->spin_unlock(&lk);
			_yield();
		}
		printf("proc_test: the mem fd is %d\n", mem_fd);
		size = vfs->read(mem_fd, buf, 1024);
		if(size < 0){
			printf("proc_test:read error while read meminfo\n");
			vfs->close(mem_fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		printf("proc_test:size:%d\ncontent:\n%s\n", size, buf);
		vfs->close(mem_fd);
	/*========================procinfo========================*/
		int proc_fd = vfs->open("/proc/0", O_RDONLY);
		if(proc_fd < 0){
			printf("proc_test:open /proc/0 error!\n");
			kmt->spin_unlock(&lk);
			_yield();
		}
		printf("proc_test: the process fd is %d\n", proc_fd);
		size = vfs->read(proc_fd, buf, 1024);
		if(size < 0){
			printf("proc_test:read error while read process\n");
			vfs->close(proc_fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		printf("proc_test:size:%d\ncontent:\n%s", size, buf);
		vfs->close(proc_fd);
		printf("proc_test:end\n\n");
		//kmt->spin_unlock(&lk);
		//_yield();
	}
	return;
}
void kv_test()
{		
	while(1){
		//kmt->spin_lock(&lk);
		char buf[1024]; int size = 0; int fd = -1;
		char name[64];	int offset = 0;	
		strcpy(name, "/forty/40c");
		if(vfs->access(name, F_OK) < 0){
			printf("kv_test:create!!!\n");
			fd = vfs->open(name, O_CREATE|O_RDWR);
		}
		else{
			printf("kv_test:not create!!\n");
			fd = vfs->open(name, O_RDWR);
		}
		printf("kv_test:fd for %s:%d\n", name, fd);
		if(fd < 0){
			printf("kv_test:open %s error!!\n", name);
			kmt->spin_unlock(&lk);
			_yield();
		}
		strcpy(buf, "forty-forty\nthis is a test for kvdb\n40404040\n");
		size = vfs->write(fd, buf, strlen(buf));
		if(size < 0){
				printf("kv_test:write %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		printf("kv_test:write %s size:%d\n", name, size);
		offset = vfs->lseek(fd, 0, SEEK_SET);
		if(offset < 0){
			printf("kv_test:lseek %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		strcpy(buf, " ");
		size = vfs->read(fd, buf, 64);
		if(size < 0){
			printf("kv_test:read %s error!!\n", name);
			vfs->close(fd);	
			kmt->spin_unlock(&lk);
			_yield();
		}
		printf("kv_test:read %s size:%d\ncontent:\n%s", name, size, buf);
		strcpy(buf, "");
		vfs->close(fd);
		printf("kv_test:end\n\n");
		//kmt->spin_unlock(&lk);
		//_yield();
	}
	return;
}
void single_thread_test()
{
	kmt->spin_lock(&lk_thread);
	kmt->create(&t1, &dev_test, NULL);
	kmt->create(&t2, &proc_test, NULL);	
	kmt->create(&t3, &kv_test, NULL);
	kmt->spin_unlock(&lk_thread);
}
void file22()
{	
while(1){
		//kmt->spin_lock(&lk);
		printf("file22:this is file22\n");
		char buf[1024]; char name[64];
		int size = 0; int fd = -1;
		strcpy(name, "/home/4040");
		if(vfs->access(name, F_OK) < 0){
			printf("file22:create!!!\n");
			fd = vfs->open(name, O_CREATE|O_RDWR);
		}
		else{
			printf("file22:not create!!\n");
			fd = vfs->open(name, O_RDWR);
		}
		
		int offset = 0;
		printf("file22:fd:%d\n", fd);
		if(fd < 0){
			printf("file22:open %s error!!\n", name);
			kmt->spin_unlock(&lk);
			_yield();
		}		
		strcpy(buf, "this is /home/4040\n");
		size = vfs->write(fd, buf, strlen(buf));
		printf("file22:size:%d\n", size);
		if(size < 0){
			printf("file22:write %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}		
		printf("file22:first write size:%d\n", size);
		offset = vfs->lseek(fd, 0, SEEK_SET);
		offset = vfs->lseek(fd, 0, SEEK_END);
		size = vfs->write(fd, buf, strlen(buf));	//写两遍
		if(size < 0){
			printf("file22:write %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		printf("file22:second write size:%d\n", size);
		offset = vfs->lseek(fd, 0, SEEK_SET);
		if(offset < 0){
			printf("file22:lseek %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		size = vfs->read(fd, buf, strlen(buf));
		if(size < 0){
			printf("file22:read %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}		
		printf("file22:read size:%d\n", size); printf("content:\n%s", buf);
		offset = vfs->lseek(fd, 0, SEEK_SET);
		if(offset < 0){
			printf("file1:lseek %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		size = vfs->read(fd, buf, strlen(buf)*2);
		if(size < 0){
			printf("file22:read %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		printf("file22:read size:%d\n", size); printf("content:\n%s", buf);
		strcpy(buf, "");
		vfs->close(fd);
		printf("file22:end\n\n");
		//kmt->spin_unlock(&lk);
		//_yield();
	}
	
	return;
}
void file2()
{	
while(1){
		//kmt->spin_lock(&lk);
		printf("file2:this is file2\n");
		char buf[1024]; char name[64];
		int size = 0; int fd = -1;
		strcpy(name, "/home/4040");
		if(vfs->access(name, F_OK) < 0){
			printf("file2:create!!!\n");
			fd = vfs->open(name, O_CREATE|O_RDWR);
		}
		else{
			printf("file2:not create!!\n");
			fd = vfs->open(name, O_RDWR);
		}
		
		int offset = 0;
		printf("file2:fd:%d\n", fd);
		if(fd < 0){
			printf("file2:open %s error!!\n", name);
			kmt->spin_unlock(&lk);
			_yield();
		}		
		strcpy(buf, "this is /home/4040\n");
		size = vfs->write(fd, buf, strlen(buf));
		printf("file2:size:%d\n", size);
		if(size < 0){
			printf("file2:write %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}		
		printf("file2:first write size:%d\n", size);
		offset = vfs->lseek(fd, 0, SEEK_SET);
		offset = vfs->lseek(fd, 0, SEEK_END);
		size = vfs->write(fd, buf, strlen(buf));	//写两遍
		if(size < 0){
			printf("file2:write %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		printf("file2:second write size:%d\n", size);
		offset = vfs->lseek(fd, 0, SEEK_SET);
		if(offset < 0){
			printf("file2:lseek %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		size = vfs->read(fd, buf, strlen(buf));
		if(size < 0){
			printf("file2:read %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}		
		printf("file2:read size:%d\n", size); printf("content:\n%s", buf);
		offset = vfs->lseek(fd, 0, SEEK_SET);
		if(offset < 0){
			printf("file2:lseek %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		size = vfs->read(fd, buf, strlen(buf)*2);
		if(size < 0){
			printf("file2:read %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		printf("file2:read size:%d\n", size); printf("content:\n%s", buf);
		strcpy(buf, "");
		vfs->close(fd);
		printf("file2:end\n\n");
		//kmt->spin_unlock(&lk);
		//_yield();
	}
	return;
}
void file11()
{	
while(1){
		//kmt->spin_lock(&lk);
		printf("file11:this is file11\n");
		char buf[1024]; char name[64];
		int size = 0; int fd = -1;
		strcpy(name, "/home/forty");
		if(vfs->access(name, F_OK) < 0){
			printf("file11:create!!!\n");
			fd = vfs->open(name, O_CREATE|O_RDWR);
		}
		else{
			printf("file11:not create!!\n");
			fd = vfs->open(name, O_RDWR);
		}
		
		int offset = 0;
		printf("file11:fd:%d\n", fd);
		if(fd < 0){
			printf("file11:open %s error!!\n", name);
			kmt->spin_unlock(&lk);
			_yield();
		}		
		strcpy(buf, "this is /home/forty\n");
		size = vfs->write(fd, buf, strlen(buf));
		printf("file11:size:%d\n", size);
		if(size < 0){
			printf("file11:write %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}		
		printf("file11:first write size:%d\n", size);
		offset = vfs->lseek(fd, 0, SEEK_SET);
		if(offset < 0){
			printf("file11:lseek %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		size = vfs->read(fd, buf, strlen(buf));
		if(size < 0){
			printf("file11:read %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}		
		printf("file11:read size:%d\n", size); printf("content:\n%s", buf);
		strcpy(buf, "");
		vfs->close(fd);
		printf("file11:end\n\n");
		//kmt->spin_unlock(&lk);
		//_yield();
	}
	return;
}
void file1()
{	
while(1){
		//kmt->spin_lock(&lk);
		printf("file1:this is file1\n");
		char buf[1024]; char name[64];
		int size = 0; int fd = -1;
		strcpy(name, "/home/forty");
		if(vfs->access(name, F_OK) < 0){
			printf("file1:create!!!\n");
			fd = vfs->open(name, O_CREATE|O_RDWR);
		}
		else{
			printf("file1:not create!!\n");
			fd = vfs->open(name, O_RDWR);
		}
		
		int offset = 0;
		printf("file1:fd:%d\n", fd);
		if(fd < 0){
			printf("file1:open %s error!!\n", name);
			kmt->spin_unlock(&lk);
			_yield();
		}		
		strcpy(buf, "this is /home/forty\n");
		size = vfs->write(fd, buf, strlen(buf));
		printf("file1:size:%d\n", size);
		if(size < 0){
			printf("file1:write %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}		
		printf("file1:first write size:%d\n", size);
		offset = vfs->lseek(fd, 0, SEEK_SET);
		if(offset < 0){
			printf("file1:lseek %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}
		size = vfs->read(fd, buf, strlen(buf));
		if(size < 0){
			printf("file1:read %s error!!\n", name);
			vfs->close(fd);
			kmt->spin_unlock(&lk);
			_yield();
		}		
		printf("file1:read size:%d\n", size); printf("content:\n%s", buf);
		strcpy(buf, "");
		vfs->close(fd);
		printf("file1:end\n\n");
		//kmt->spin_unlock(&lk);
		//_yield();
	}
	return;
}

void multi_thread_test()
{
	kmt->spin_lock(&lk_thread);
	kmt->create(&t4, &file2, NULL);kmt->create(&t5, &file22, NULL);
	kmt->create(&t6, &file1, NULL);kmt->create(&t7, &file11, NULL);
	kmt->spin_unlock(&lk_thread);
}
void test_file()
{
	kmt->spin_init(&lk, "filetest_lk");
	kmt->spin_init(&lk_thread, "multithread_lk");
	single_thread_test();
	//multi_thread_test();
	return;
}


