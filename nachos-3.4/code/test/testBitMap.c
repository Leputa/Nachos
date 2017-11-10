#include "syscall.h"
int mystrlen(char *buffer)
	 {
	         int i;
	         for(i=0;i<500;i++)
	         {
	                 if(buffer[i]==0)
	                         return i;
	         }
	         return -1;
	 }


void main()
{
	char *name = "testBitMap";
	char *str1 = "hello,world!\n";
	char *str2 = "who are you!\n";
	char *str3 = "What are you doing now?\n";
	char *str4 = "I am studing Nachos coding!\n";
	char buffer[255];
	int i;
	OpenFileId fd;
	Create(name);
	fd = Open(name);
	Write(str1,mystrlen(str1),fd);
	Write(str2,mystrlen(str1),fd);
	Write(str3,mystrlen(str1),fd);
	Write(str4,mystrlen(str1),fd);
	if(1==1)
		Write("guguday\n",13,fd);
	if(1==1)
		Write("jijiji\n",13,fd);
	if(1==1)
		Write("yiyiyi\n",13,fd);
	if(1==1)
		Write("biubiubiu~\n",13,fd);
	if(1==1)
		Write("hahaha\n",13,fd);
	if(1==1)
		Write("guaguagua\n",13,fd);
	Close(fd);

    Halt();
}
