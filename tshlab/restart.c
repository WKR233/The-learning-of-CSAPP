#include "csapp.h"

sigjmp_buf buf;

void handler(int sig)
{
    siglongjmp(buf,1);
}

int main()
{
    if(!sigsetjmp(buf,1)){
        signal(SIGINT,handler);
        sio_puts("starting\n");
    }
    else
        sio_puts("restarting\n");

    while(1){
        sleep(1);
        sio_puts("processing...\n");
    }
    exit(0);
}