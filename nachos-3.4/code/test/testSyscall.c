#include "syscall.h"

OpenFileId fd1,fd2;
int result;
char buffer[32];

int main(){
    Create("WriteSysCall.txt");
    fd1=Open("ReadSysCall.txt");
    fd2=Open("WriteSysCall.txt");
    result=Read(buffer,32,fd1);
    Write(buffer,result,fd2);
    Close(fd1);
    Close(fd2);
    Halt();
}
