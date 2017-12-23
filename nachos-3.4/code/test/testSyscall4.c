#include "syscall.h"

void func(){
    Create("testFile1");
}

int main(){
    Create("testFile2");
    Fork(func);
}
