#include <os.h>
#include<libc.h>

#define STK_SZ 256*1024
#define FC_SZ 32
#define T_max 20


typedef struct thread thread_t;
typedef struct spinlock spinlock_t;
typedef struct semaphore sem_t;
static void kmt_init();
static int create(thread_t *thread, void (*entry)(void *arg), void *arg);
static void teardown(thread_t *thread);
static thread_t* schedule();
static void spin_init(spinlock_t *lk, const char *name);
static void spin_lock(spinlock_t *lk);
static void spin_unlock(spinlock_t *lk);
static void sem_init(sem_t *sem, const char *name, int value);
static void sem_wait(sem_t *sem);
static void sem_signal(sem_t *sem);

extern thread_t work[T_max];
extern int thread_cnt;
extern int current_id;
extern int lock_cnt;
int lock_cnt;


MOD_DEF(kmt) 
{
	.init = kmt_init,
	.create = create,
	.teardown = teardown,
	.schedule = schedule,
	.spin_init = spin_init,
	.spin_lock = spin_lock,
	.spin_unlock = spin_unlock,
	.sem_init = sem_init,
	.sem_wait = sem_wait,
	.sem_signal = sem_signal,
};

spinlock_t sem_lk;
thread_t work[T_max];
int thread_cnt;
int current_id;

static void kmt_init()
{
	spin_init(&sem_lk, "sem_lk");
	for(int i = 0; i<T_max; i++)
		work[i].id = -1;
	current_id = 0;
	thread_cnt = 0;
	lock_cnt = 0;
}
  /*===================================*/
 /*==========deal with thread=========*/
/*===================================*/
spinlock_t lk;
static int create(thread_t *thread, void (*entry)(void *arg), void *arg)
{
	spin_lock(&lk);
	//printf("thread id:%d\n", thread->id);
	void *fence1_addr = pmm->alloc(FC_SZ);
	void *addr = pmm->alloc(STK_SZ);
	void *fence2_addr = pmm->alloc(FC_SZ);
	if(addr && fence1_addr && fence2_addr){
		thread->id = thread_cnt;
		thread->fence1 = fence1_addr;
		thread->stack = addr;
		thread->fence2 = fence2_addr;
		int id = thread->id;
		for(int i = 0; i<32; i++){
			thread->fence1[i] = id;
			thread->fence2[i] = id;
		}
		_Area stack;
		stack.start = thread->stack; stack.end = thread->stack + STK_SZ;
		thread->thread_reg = _make(stack, entry, arg);
		work[thread_cnt] = *thread;
		thread_cnt++;
		printf("in kmt thread_cnt:%d\n", thread_cnt);
		/*========create proc thread info========*/
		
		char path[64];
		strcpy(path, "/proc/"); strcat(path, itoa(thread->id));
		int fd = vfs->open(path, O_CREATE|O_RDWR);
		int offset = 0;
		char buf[128];
		strcpy(buf, "id: "); 
		offset = strlen(buf);
		if(vfs->write(fd, buf, strlen(buf)) < 0){
			printf("write thread %d proc file error\n", thread->id);
			return -1;
		}
		
		vfs->lseek(fd, offset, SEEK_SET);
		strcpy(buf, itoa(thread->id));
		offset += strlen(buf);
		if(vfs->write(fd, buf, strlen(buf)) < 0){
			printf("write thread %d proc file error\n", thread->id);
			vfs->close(fd);
			pmm->free(path); pmm->free(buf);
			return -1;
		}	
		
		vfs->lseek(fd, offset, SEEK_SET);	
		strcpy(buf, "\n");
		offset += strlen(buf);
		if(vfs->write(fd, buf, strlen(buf)) < 0){
			printf("write thread %d proc file error\n", thread->id);
			vfs->close(fd);
			pmm->free(path); pmm->free(buf);			
			return -1;
		}
			
		vfs->lseek(fd, offset, SEEK_SET);	
		strcpy(buf, "stack size: ");
		offset += strlen(buf);
		if(vfs->write(fd, buf, strlen(buf)) < 0){
			printf("write thread %d proc file error\n", thread->id);
			vfs->close(fd);
			pmm->free(path); pmm->free(buf);			
			return -1;
		}
			
		vfs->lseek(fd, offset, SEEK_SET);		
		strcpy(buf, itoa(STK_SZ));
		offset += strlen(buf);
		if(vfs->write(fd, buf, strlen(buf)) < 0){
			printf("write thread %d proc file error\n", thread->id);
			vfs->close(fd);
			pmm->free(path); pmm->free(buf);			
			return -1;
		}
		
		vfs->lseek(fd, offset, SEEK_SET);
		strcpy(buf, "\n");
		offset += strlen(buf);
		if(vfs->write(fd, buf, strlen(buf)) < 0){
			printf("write thread %d proc file error\n", thread->id);
			vfs->close(fd);
			pmm->free(path); pmm->free(buf);			
			return -1;
		}	
		
		vfs->lseek(fd, offset, SEEK_SET);			
		strcpy(buf, "stack start: ");
		offset += strlen(buf);
		if(vfs->write(fd, buf, strlen(buf)) < 0){
			printf("write thread %d proc file error\n", thread->id);
			vfs->close(fd);
			pmm->free(path); pmm->free(buf);			
			return -1;
		}
		
		vfs->lseek(fd, offset, SEEK_SET);
		strcpy(buf, itoa((int)stack.start));
		offset += strlen(buf);
		if(vfs->write(fd, buf, strlen(buf)) < 0){
			printf("write thread %d proc file error\n", thread->id);
			vfs->close(fd);
			pmm->free(path); pmm->free(buf);			
			return -1;
		}
		
		vfs->lseek(fd, offset, SEEK_SET);
		strcpy(buf, " ");
		offset += strlen(buf);
		if(vfs->write(fd, buf, strlen(buf)) < 0){
			printf("write thread %d proc file error\n", thread->id);
			vfs->close(fd);
			pmm->free(path); pmm->free(buf);			
			return -1;
		}	
		
		vfs->lseek(fd, offset, SEEK_SET);			
		strcpy(buf, "stack end: ");
		offset += strlen(buf);
		if(vfs->write(fd, buf, strlen(buf)) < 0){
			printf("write thread %d proc file error\n", thread->id);
			vfs->close(fd);
			pmm->free(path); pmm->free(buf);			
			return -1;
		}
		
		vfs->lseek(fd, offset, SEEK_SET);
		strcpy(buf, itoa((int)stack.end));
		offset += strlen(buf);
		if(vfs->write(fd, buf, strlen(buf)) < 0){
			printf("write thread %d proc file error\n", thread->id);
			vfs->close(fd);
			pmm->free(path); pmm->free(buf);			
			return -1;
		}
		vfs->lseek(fd, offset, SEEK_SET);						
		strcpy(buf, "\n");
		offset += strlen(buf);
		if(vfs->write(fd, buf, strlen(buf)) < 0){
			printf("write thread %d proc file error\n", thread->id);
			vfs->close(fd);
			pmm->free(path); pmm->free(buf);			
			return -1;
		}		
		vfs->close(fd);
		//pmm->free(path); pmm->free(buf);	
		spin_unlock(&lk);					
		return 0;
	}
	printf("error while alloc for thread stack in create\n");
	spin_unlock(&lk);
	return -1;
}
static void teardown(thread_t *thread)
{
	void* addr = thread->stack;
	void* fence1_addr = thread->fence1;
	void* fence2_addr = thread->fence2;
	if(addr && fence1_addr && fence2_addr){
		if(current_id == thread->id){
			current_id = (current_id+1)%thread_cnt;
		}
		work[thread->id].id = -1;
		for(int i = thread->id; i<thread_cnt-1; i++)
			work[i] = work[i+1];
		work[thread_cnt].id = -1;
		thread_cnt--;
		pmm->free(addr); pmm->free(fence1_addr); pmm->free(fence2_addr);
	}
	return;
}
static thread_t* schedule()
{		
	int old_id = current_id;
	current_id = (current_id+1)%thread_cnt;
	printf("old id:%d current id:%d\n", old_id, current_id);
	//printf("work eip:0x%08x\n", work[current_id].thread_reg->eip);
	return &work[old_id];
}
  /*===================================*/
 /*=========deal with spinlock========*/
/*===================================*/
static void spin_init(spinlock_t *lk, const char *name)
{
	lk->locked = 0;	//未被占用的锁
	int len = strlen(name);
	strncpy(lk->name, name, len);
	return;
}
static void spin_lock(spinlock_t *lk)
{
	if(_intr_read())
		_intr_write(0);		//关中断
	lock_cnt++;
	while(_atomic_xchg(&lk->locked, 1) != 0);
}
static void spin_unlock(spinlock_t *lk)
{
	_atomic_xchg(&lk->locked, 0);
	lock_cnt--;
	if(lock_cnt == 0)
		_intr_write(1);
}
  /*===================================*/
 /*========deal with semaphore========*/
/*===================================*/

static void sem_init(sem_t *sem, const char *name, int value)
{
	sem->count = value;
	int len = strlen(name);
	strncpy(sem->name, name ,len);
	sem->queue = pmm->alloc(sizeof(struct queue_node));
	if(sem->queue == NULL){
		printf("error while alloc for sem->queue in sem_init\n");
		_halt(1);
	}
	sem->queue->if_in = 0; sem->queue->next = NULL; sem->queue->prev = NULL;
	return;
}
static void sem_wait(sem_t *sem)
{
	spin_lock(&sem_lk);
	sem->count--;
	if(sem->count < 0){
		sem->count++;
		struct queue_node* current_node = pmm->alloc(sizeof(struct queue_node));
		if(sem->queue == NULL){
			printf("error while alloc for sem->queue current_node in sem_wait\n");
			_halt(1);
		}		
		current_node = sem->queue;	
		for(; current_node->next; current_node = current_node->next){
			if(!current_node->if_in){	//找到最前面的那个node
				break;
			}
		}
		if(!current_node->if_in){
			current_node->if_in = 1;
		}
		else{
			struct queue_node* add_node = pmm->alloc(sizeof(struct queue_node));
			if(add_node == NULL){
				printf("error while alloc for add_node in sem_wait\n");
				_halt(1);
			}
			add_node->prev = NULL; add_node->next = sem->queue; add_node->if_in = 1;
			sem->queue->prev = add_node;
			sem->queue = add_node;		
			//!@#$printf("add node:0x%08x\n", add_node);	
		}	
		struct queue_node* last_node = pmm->alloc(sizeof(struct queue_node));
		if(last_node == NULL){
			printf("error while alloc for last_node in sem_wait\n");
			_halt(1);
		}		
		last_node = sem->queue;
		while(last_node->next){
			if(last_node->if_in)
				break;
			last_node = last_node->next;
		}
		spin_unlock(&sem_lk);
		while(last_node->if_in){
			printf("");	//我也不知道为什么要一个printf才可以……
		}
		spin_lock(&sem_lk);
		sem->count--;
		if(last_node->prev || last_node->next){
			if(last_node->prev){
				last_node->prev->next = last_node->next;
				last_node->prev = NULL;
			}
			if(last_node->next){
				last_node->next->prev = last_node->prev;
				last_node->next = NULL;
			}
		}
		pmm->free(last_node);
	}
	spin_unlock(&sem_lk);
	
	return;
}
static void sem_signal(sem_t *sem)
{
	spin_lock(&sem_lk);
	sem->count++;
	struct queue_node* last_node = pmm->alloc(sizeof(struct queue_node));
	if(last_node == NULL){
		printf("error while alloc for last_node in sem_signal\n");
		_halt(1);
	}	
	int if_occupied = 0;
	last_node = sem->queue;
	for(;last_node; last_node = last_node->next){
		if(last_node->if_in){
			if_occupied = 1;
			break;
		}
	}
	if(if_occupied){
		last_node = sem->queue;
		while(last_node->next){
			if(last_node->if_in)
				break;
			last_node = last_node->next;
		}
		last_node->if_in = 0;
	}
	spin_unlock(&sem_lk);
	return;
}

