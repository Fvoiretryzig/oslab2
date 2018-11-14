#include<libc.h>
#include<stdarg.h>


static int cnt;
static int if_left_align;
static int if_align;
static int width ;
static char seperate ;

void align()
{
	for(int i = 0; i<width; i++){
		_putc(seperate);
		cnt++;
	}
}
void p_str(char *s)
{
	char* temp = "(null)";
	if(s == NULL){
		while(*temp){
			_putc(*temp++);
		}
	}
	else{
		while(*s){
			_putc(*s++);
			cnt++;
		}	
	}

	return;
}
void p_int(int num, int rank)
{
	if(!num && rank)
		return;
	else
		p_int(num/10, ++rank);
	if(!num && !rank){
		_putc('0');
		cnt++;
		return;
	}
	else{
		_putc((char)(num%10 + '0'));
		cnt++;
	}
	return;
}
void p_hex(int num, int rank)
{
	if(!num && rank)
		return;
		
	else
		p_hex(num/16, ++rank);
	if(!num && !rank){
		_putc('0');
		cnt++;
		return;
	}
	else if(num%16 < 10){
		_putc((char)(num%16 + '0'));
		cnt++;
	}
	else{
		_putc((char)(num%16 + 'a' - 10));
		cnt++;
	}
	return;
}
int printf(const char* fmt, ...)
{
	cnt = 0;
	va_list arg;
	va_start(arg, fmt);
	
    int  arg_int = 0;
    char* arg_str = NULL;
    char arg_ch = 0;
    

	
    while(*fmt){
    	if(*fmt == '%'){
    		fmt++;
    		if_left_align = 0; seperate = ' '; width = 0; if_align = 0;
    		if(*fmt == '-'){
    			if_left_align = 1;
    			fmt++;
    		}
    		if(*fmt == '0'){
    			seperate = '0';
    			fmt++;
    		}
    		if(*fmt >= '0' && *fmt <= '9'){
    			if_align = 1;
    			width = *fmt - '0';
    			fmt++;
    		}	
    		switch(*fmt){
    			case '%':
    				_putc('%');
    				cnt++;
    				break;
    			case 'd':
    			case 'i':
    				arg_int = va_arg(arg, int);
    				if(arg_int < 0){
    					_putc('-');
    					arg_int = -arg_int;
    				}
    				if(if_align)
    				{
    					int temp = arg_int; int len = 0;
    					if(temp == 0)	//0也占了一个位置
    						len++;    					
	    				while(temp){	//求出字符串长度看要不要打印
	    					temp /=10;
	    					len++;
	    				}
	    				width -= len;
	    				if(width > 0){
	    					if(if_left_align){
	    						p_int(arg_int, 0);
	    						align();
	    					}
	    					else{
	    						align();
	    						p_int(arg_int, 0);
	    					}
	    				}
	    				else
	    					p_int(arg_int, 0);
    				}
					else
	    				p_int(arg_int, 0);
    				break;
    			case 'c':
    				arg_ch = va_arg(arg, int);
    				if(if_align){
    					width--;	//字符的长度只有一个
    					if(width > 0){
    						if(if_left_align){
    							_putc(arg_ch);
    							align();
    						}	
    						else{
    							align();
    							_putc(arg_ch);
    						}
    					}
    					else
    						_putc(arg_ch);
    					cnt++;
    				}
    				else
    					_putc(arg_ch);
    				cnt++;
    				break;
    			case 's':
    				arg_str = va_arg(arg, char*);
    				if(if_align){
    					int len = strlen(arg_str);
    					width -= len;
    					if(width > 0){
    						if(if_left_align){
    							p_str(arg_str);
    							align();
    						}
    						else{
    							align();
    							p_str(arg_str);
    						}
    					}
    					else
    						p_str(arg_str);
    				}
    				else
	    				p_str(arg_str);
    				break;
    			case 'x':
    			case 'X':
	    			arg_int = va_arg(arg, int);
	    			if(arg_int < 0){
	    				_putc('-');
	    				arg_int = -arg_int;
	    			}
    				if(if_align){
    					int temp = arg_int; int len = 0;
    					if(temp == 0)	//0也占了一个位置
    						len++;    					
	    				while(temp){	//求出字符串长度看要不要打印
	    					temp /=16;
	    					len++;
	    				}
	    				width -= len;
	    				if(width > 0){
	    					if(if_left_align){
	    						p_hex(arg_int, 0);
	    						align();
	    					}
	    					else{
	    						align();
	    						p_hex(arg_int, 0);
	    					}
	    				} 
	    				else
	    					p_hex(arg_int, 0);   					
    				}
					else
	    				p_hex(arg_int, 0);
    				break;
    			default:
    				break;
    		}
    		fmt++;
    	}
    	else{
    		_putc(*fmt++);
    		cnt++;
    	}	
    }
	va_end(arg);
	return cnt;
}
int sprintf(char* out, const char* format, ...);
int snprintf(char* s, size_t n, const char* format, ...);
