#include <atomic>
#include <thread>
#include <iostream>

using namespace std;

atomic<int> x(0);
atomic<int> y(0);

void thread0_relaxed() {
    x.store(1, memory_order_relaxed);
}

void thread1_relaxed() {
    cout << x.load(memory_order_relaxed);
    cout << y.load(memory_order_relaxed);
}

void thread2_relaxed() {
    cout << y.load(memory_order_relaxed);
    cout << x.load(memory_order_relaxed);

}
void thread3_relaxed() {
    y.store(1, memory_order_relaxed);
}
int main() {
    thread t0(thread0_relaxed);
    thread t1(thread1_relaxed);
    thread t2(thread1_relaxed);
    thread t3(thread1_relaxed);

    t0.join();
    t1.join();
    t2.join();
    t3.join();

    return 0;
}