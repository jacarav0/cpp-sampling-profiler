
void hot_funct() {
    volatile double x = 0;
    for (int i = 0; i < 50000000; i++) x += i * 0.5;
}

void cold_funct() {
    volatile double x = 0;
    for (int i = 0; i < 5000000; i++) x += i * 0.5;
}

int recursive(int n) {
    if (n <= 0) return 0;
    volatile double x = 0;
    for (int i = 0; i < 1000000; i++) x += i;
    return n + recursive(n - 1);
}
