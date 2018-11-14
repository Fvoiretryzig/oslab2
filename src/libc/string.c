#include<libc.h>
#include <vfs.h>

mountpath_t* kvfs_p;
void *memset(void *b, int c, size_t n)
{
	if(b == NULL || n < 0){
		printf("null in memset\n");
		return NULL;
	}
	char *temp_b = (char *)b;
	while(n--)
		*temp_b++ = c;
	return b;
}
void *memcpy(void *dst, const void *src, size_t n)
{
	if(dst == NULL || src == NULL){
		printf("null in memcpy\n");
		return NULL;
	}
	char *temp_src = (char *)src;
	char *temp_dst = (char *)dst;
	if(((temp_src <= temp_dst) && (temp_dst <temp_src+n)) ||
	   ((temp_src >= temp_dst) && (temp_src <temp_src+n))){
		printf("this is overlap in memcpy");//printf("this is no solution\n");
		return NULL;
	}
    while(n--){
        *temp_dst = *temp_src;
        temp_dst++;
        temp_src++;
    }
    return dst;			
}
size_t strlen(const char* s)
{
	const char *temp = s;
	while(*temp++);
	return (temp-s-1);
}
char *strcpy(char *dst, const char *src)
{
	char *temp_dst = dst;
	while((*temp_dst++ = *src++)!='\0');
	//printf("strcpy:kvfs_p->fs->inode[0]:%s inode[1]:%s\n", kvfs_p->fs->inode[0]->name,kvfs_p->fs->inode[1]->name);
	return dst;
}
char *strncpy(char *dst, const char *src, size_t n)
{
	int temp_n = n;
	char *temp_dst = dst;
	while(n-- && (*temp_dst++ = *src++));
	if(temp_n)
		while(--temp_n)
			*temp_dst = '\0';
	return dst;
}
int strcmp ( const char* s1, const char* s2 )  
{  
    int ret = 0 ;  
    while(*s1 == *s2 && *s2){
    	s1++; s2++;
    }
   	int tmp = *s1 -*s2;
   	if(tmp > 0)
   		ret = 1;
   	else if(tmp < 0)
   		ret = -1;
    return ret;  
}  
int strncmp(const char *s1, const char *s2, size_t n)
{
	int ret = 0;
	while(*s1 == *s2 && *s2 && n--){
		s1++; s2++;
	}
	if(n){
		int temp = *s1 - *s2;
	    if(temp > 0)
	    	ret = 1;
	    else if(temp < 0)
	    	ret = -1; 	
	}
    return ret;  		
}
char * strcat(char * dst, const char * src)
{
	char *tmp = dst;
	while(*tmp)
		tmp++;
	while ((*tmp++ = *src++) != '\0');
	return tmp;
}
char * strncat(char * front, const char * back, size_t count)
{
   char *start = front;
   while (*front++);
   while (count--){
       if (!(*front++ = *back++))
           return(start);   
   }
    *front ='\0';
   return(start);
}

