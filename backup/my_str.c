#include<stdio.h>

#define N_FOR_STRNCPY	5

/*return a string's length*/
int x_strlen(const char *str)
{
    int len = 0;
	while(*str != '\0')
	{
		str++;
	    len++;
	}
	return len;
}

/*copy sou to des, and return the poiter to des */  
/*因为des是指向与内存的起始地址，而后面的内容也都已经是改变过了，\
所以这时候是可以直接从头打印的。而tmp是什么情况还是需要再试验一下。*/
char *x_strcpy (char *des,const char *src)	
{
    char *ret = des;
	while(*src != '\0')
	{
		*des++ = *src++;
	}
	
	return ret;
}

/*copy src to des for n number, and return the poiter to des */
char *x_strncpy(char *dest, const char *src, int n)
{
    char *ret = dest;
	int i = 0;
	
	if(x_strlen(src) < n)
	{
		dest =x_strcpy(dest, src);
	}
	else
	{
		for(i=0; i<n;i++)
		{
			*dest++ = *src++;
		}
		*(dest++) = '\0';
    }
    return ret;	//因为拷贝是dest会移动，而最终要返回的是拷贝后字符串的起始地址，因此要先保存dest的地址，便于最终返回。
}

/*copare two str to see which one is bigger, and return <0 or >0*/
 int x_strcmp(const char *str1, const char *str2)
 {
    int ret = 0;
	while(*str1 && *str2)
	{
	if(*str1 == *str2)
	{
		str1++;
		str2++;
	    ret = 0; 
	}
	else 
	{
		return ret = (*str1 - *str2); 
	}
	}
	return ret;
 }

/*copare two str's first maxlen number to see which one is bigger, and return <0 or >0*/
int x_strncmp(const char *str1, const char *str2, int maxlen)
{
	int ret = 0;
	//if(!maxlen){return 0;}		//参考网络，判断maxlen是否是无符号整型变量
	while(maxlen-- && *str1 && *str2)
	{
		if(*str1 == *str2)
		{
			str1++;
			str2++;
			ret = 0; 
		}
		else 
		{
			ret = (*str1 - *str2);
			return ret;
		}
	}
	return ret;
}

/*copare two str's first maxlen number to see which one is bigger, and return <0 or >0*/
char *x_strchr(char *str, char c)
{
	char *ret = str;
	while (*str)
	{
		if(*str++ == c)						
		{
			ret = str;
			return ret;
		}
		else
		{
			ret = NULL;
		}
	}
	return ret;
}

/*copare two str's first maxlen number to see which one is bigger, and return <0 or >0*/
char *x_strstr( char *haystack, const char *needle)
{
	char *ret = haystack;
	
	while(*haystack)
	{
		while(*haystack != *needle)
		{
			haystack++;
			ret = NULL;
		}
		
		ret = haystack;
		
		while(*needle != '\0')
		{
			if(*haystack++ != *needle++)
			{
				ret = NULL;
			}
		}
	}
}

/*copare two str's first maxlen number to see which one is bigger, and return <0 or >0*/
char *x_strstr_2( char *haystack, const char *needle)
{
	char *ret = haystack;
	int num = 0;
	while(*haystack)
	{	
		for (num=0; *(haystack+num)==*(needle+num); num++)
		{
			if(!(*(needle+num+1)))
			{
				ret = haystack;
				return ret;
			}
		}
		haystack++;
	}
	ret = NULL;
	return ret;
		
}

void main(int argc, char *argv[])
{
	
	
	char src[] = {"I am Chinese."};
	char dest[] = {"I love China."}; 
	
	int length = 0;
	length = x_strlen(src);
	printf("test for x_strlen.length = %d\n",length);
	
	char *after_str_1 ;
	after_str_1 = x_strcpy(dest, src);
	printf("test for x_strcpy.after_str_1 = %s\n",after_str_1);
	
	char *after_str_2 ;
	after_str_2 = x_strncpy(dest, src, N_FOR_STRNCPY);
	printf("test for x_strncpy.after_str_2 = %s\n",after_str_2);
	
	int cmp ;
	cmp = x_strcmp("1234", "123m");
	printf("test for x_strcmp.cmp = %d\n",cmp);
	
	int cmp_n ;
	cmp_n = x_strncmp("123456", "123m ch", 4);
	printf("test for x_strncmp.cmp_n = %d\n",cmp_n);
	
	char *after_strchr ;
	char c = 'a';
	after_strchr = x_strchr("I am Chinese", c);
	printf("test for x_strchr.after_strchr = %s\n",after_strchr);
	
	char *after_strstr ;
	after_strstr = x_strstr_2("I am Chinese", "hi");
	printf("test for x_strchr.after_strstr = %s\n",after_strstr);
	
}