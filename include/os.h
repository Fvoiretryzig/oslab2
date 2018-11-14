#ifndef __OS_H__
#define __OS_H__

#include <kernel.h>

struct thread	
{
	int id;
	uint8_t* fence1;
	uint8_t* stack;
	uint8_t* fence2;	
	_RegSet *thread_reg;
};
struct queue_node
{
	int if_in;
	struct queue_node* next;
	struct queue_node* prev;
};
struct thread_node
{
	struct thread* t;
	struct thread_node* next;
	struct thread_node* prev;
};
struct spinlock
{
	int locked;	//判断锁是否被占用了
	char name[64];
};

struct semaphore
{
	int volatile count;
	struct queue_node* volatile queue;
	char name[64];
};

static inline void puts(const char *p) {
  for (; *p; p++) {
    _putc(*p);
  }
}

#endif
