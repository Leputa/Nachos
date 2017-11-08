#include "syscall.h"
#define M 16
#define N 16
void main(){
    int num[M][N];
    int i,j;
    for(i=0;i<M;i++){
        for(j=0;j<N;j++){
            num[i][j] = 0;
        }
    }
    Halt();
}
