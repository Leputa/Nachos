#include <time.h>
#include <stdio.h>
#include <string.h>

int main(){
    time_t timep;
    time(&timep);
    printf("%s",asctime(gmtime(&timep)));
    printf("%d\n",strlen(asctime(gmtime(&timep))));
    return 0;
}
