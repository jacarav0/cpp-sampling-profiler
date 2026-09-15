#include <cstdio>
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <cstdint>

volatile sig_atomic_t sample_count = 0;
static const int MAX_SAMPLES = 4096;
static const int MAX_FRAMES = 64;
static uintptr_t samples[MAX_SAMPLES][MAX_FRAMES];
static int frame_counts[MAX_SAMPLES];

static int walk_stack(uintptr_t fp, uintptr_t* out, int max_frames) {
    int n = 0;
    uintptr_t prev = 0;

    while (fp && n < max_frames) {
        if (fp <= prev) break;          
        if (fp & 0x7) break;          

        uintptr_t ret = *(uintptr_t*)(fp + 8);
        if (!ret) break;

        out[n++] = ret;

        prev = fp;
        fp = *(uintptr_t*)fp;           
    }
    return n;
}
void handler(int sig, siginfo_t* info, void* context) {
    (void)sig;
    (void)info;
    if(sample_count >=MAX_SAMPLES) return;
    ucontext_t* uc = (ucontext_t*)context;
    uintptr_t fp = uc->uc_mcontext.gregs[REG_RBP];
    frame_counts[sample_count] = walk_stack(fp, samples[sample_count], MAX_FRAMES);
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
    printf("samples: %d\n", (int)sample_count);
    for (int i = 0; i < 5 && i < sample_count; i++) {
    printf("sample %d (%d frames): ", i, frame_counts[i]);
    for (int j = 0; j < frame_counts[i]; j++) {
        printf("%lx ", (unsigned long)samples[i][j]);
    }
    printf("\n");
}
    


}




