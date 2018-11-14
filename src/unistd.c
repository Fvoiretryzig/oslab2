#include<unistd.h>
#include<os.h>

int* program_break;
//应该无关物理地址虚拟地址的转换……吧
int brk(void *addr)
{
	//if((uint32_t)addr >= (uint32_t)program_break && (uint32_t)addr < (uint32_t)_heap.end){
	if((uint32_t)addr >= (uint32_t)_heap.start && (uint32_t)addr < (uint32_t)_heap.end){
		program_break = addr;
		return 0;
	}
	return -1;
}
void *sbrk(intptr_t increment)	
{
	if(!increment)
		return program_break;
	int* old_program_break = program_break;
	if((uint32_t)(increment + old_program_break) < (uint32_t)_heap.end){
		program_break += increment;
		return old_program_break;
	}
	return (void*)-1;
}
