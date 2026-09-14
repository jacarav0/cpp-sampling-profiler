#include <cstdio>
#include <iostream>
#include <signal.h>
#include <sys/time.h>

volatile sig_atomic_t sample_count = 0;


void handler(int sig, siginfo_t* info, void* context) {
    (void)sig;
    (void)info;
    (void)context;
    sample_count++;
}
void hot_funct(){
    volatile double x = 0;
    for(int i{};i<50000000;i++) x+=i * 0.5;
}

int main(){
    struct sigaction sa;
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPROF, &sa, nullptr) == -1) { perror("sigaction"); return 1; }
    
    struct itimerval timer = {};
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 10000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 10000;
    setitimer(ITIMER_PROF, &timer, nullptr);
        printf("samples: %d\n", sample_count);

    for (int i = 0; i < 10; i++){
        hot_funct();
    }
        printf("samples: %d\n", sample_count);


}



