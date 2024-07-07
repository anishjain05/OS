#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(){
    pid_t rc=vfork();
    if(rc<0){
        fprintf(stderr,"Vfork failed\n");
        return 1;
    }
    else if(rc==0){
        printf("Child process\n");
        int prod=1;
        for(int i=2;i<=4;i++){
            prod*=i;
        }
        printf("Factorial of 4 is %d\n",prod);
    }
    else{
        printf("Parent Process\n");
        wait(NULL);
        int n = 16;
        int fib[n];
        fib[0] = 0;
        fib[1] = 1;
        for (int i = 2; i < n; i++) {
            fib[i] = fib[i - 1] + fib[i - 2];
        }
        printf("Fibonacci series up to %d: ", n);
        for (int i = 0; i < n; i++) {
            printf("%d ", fib[i]);
        }
        printf("\n");
    }
    return 0;
}