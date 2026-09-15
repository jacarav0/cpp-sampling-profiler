#include <cstdio>
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <cstdint>
#include <sstream>
#include <map>
#include <memory>
#include <string>
#include <ucontext.h>
#include "workload.h"
volatile sig_atomic_t sample_count = 0;
static const int MAX_SAMPLES = 4096;
static const int MAX_FRAMES = 64;
static uintptr_t samples[MAX_SAMPLES][MAX_FRAMES];
static int frame_counts[MAX_SAMPLES];
static std::map<uintptr_t,std::string> symbolCache;

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
static std::string resolveAddress(const std::string& binaryPath, uintptr_t addr) {
    std::stringstream cmd;
    cmd << "addr2line -e " << binaryPath << " -f -C 0x" << std::hex << addr;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.str().c_str(), "r"), pclose);
    if (!pipe) return "??";

    char buf[512];
    if (!fgets(buf, sizeof(buf), pipe.get())) return "??";

    std::string result(buf);
    if (!result.empty() && result.back() == '\n') result.pop_back();
    return result.empty() ? "??" : result;
}
static const std::string& symbolize(const std::string& binaryPath, uintptr_t addr) {
    auto it = symbolCache.find(addr);
    if (it == symbolCache.end()) {
        it = symbolCache.emplace(addr, resolveAddress(binaryPath, addr)).first;
}
return it->second;
}
void handler(int sig, siginfo_t* info, void* context) {
    (void)sig;
    (void)info;
    if(sample_count >=MAX_SAMPLES) return;
    ucontext_t* uc = (ucontext_t*)context;
    uintptr_t fp = uc->uc_mcontext.gregs[REG_RBP];
    samples[sample_count][0] = uc->uc_mcontext.gregs[REG_RIP];
    int n = walk_stack(fp, &samples[sample_count][1], MAX_FRAMES - 1);
    frame_counts[sample_count] = n + 1;
    sample_count++;
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
        setitimer(ITIMER_PROF, &timer, nullptr);

        for (int i = 0; i < 10; i++) {
            hot_funct();
            cold_funct();
            recursive(10);
            recure(10);
        }
        std::map<std::string, int> counts;
            if (setitimer(ITIMER_PROF, &timer, nullptr) == -1) { perror("setitimer"); return 1; }
    for (int i = 0; i < sample_count; i++) {
        std::string stack;
        for (int j = frame_counts[i] - 1; j >= 0; j--) {
            if (!stack.empty()) stack += ";";
            stack += symbolize("./profiler", samples[i][j]);
        }
        counts[stack]++;
    }

    for (const auto& kv : counts) {
        printf("%s %d\n", kv.first.c_str(), kv.second);
    }

    return 0;
}





