#include <stdio.h> 
#include <signal.h> 
#include <stdlib.h> 

void handler(int signum){
    printf("Got Signal! Number = %d\n", signum);
    int x = 0;
    int y = 5;
    y /= x;
    exit(0);
}

int main(){
    signal(SIGFPE, handler);
    int x = 120;
    for (int i = 6; i >= 0; --i){
        printf("%d\n", x / i);
    }
    printf("Magic Number!\n");
    return 0;
}