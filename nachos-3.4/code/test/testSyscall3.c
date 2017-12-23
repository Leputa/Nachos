#include "syscall.h"
int main(){
    int id=Exec("../test/halt");
    Join(id);
}
