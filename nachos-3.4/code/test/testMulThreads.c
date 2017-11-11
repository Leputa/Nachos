#include "syscall.h"
#define M 16
void main(){
    int num[M];
    int i,j;
    for(i=0;i<M;i++){
        num[i] = 0;
    }
    Halt();
}
