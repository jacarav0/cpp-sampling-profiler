#include <cstdio>

// Should dominate the profile
void hot_function() {
    volatile double x = 0;
    for (int i = 0; i < 50000000; i++) x += i * 0.5;
}

// Should be a thin sliver
void cold_function() {
    volatile double x = 0;
    for (int i = 0; i < 100000; i++) x += i * 0.5;
}

// Gives you stack depth to walk
int recursive(int n) {
    if (n <= 0) return 0;
    volatile double x = 0;
    for (int i = 0; i < 100000; i++) x += i;
    return n + recursive(n - 1);
}

int main() {
    for (int i = 0; i < 10; i++) {
        hot_function();
        cold_function();
        recursive(10);
    }
    printf("done\n");
    return 0;
}