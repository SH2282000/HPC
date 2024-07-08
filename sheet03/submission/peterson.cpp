#include <atomic>
#include <iostream>

using namespace std;

class Peterson
{
private:
    atomic<bool> interested[2];
    atomic<int> turn;

public:
    Peterson()
    {
        interested[0].store(0);
        interested[1].store(0);
        turn.store(0);
    }

    void acquire(int id)
    {
        int other = 1 - id;
        interested[id].store(true, memory_order_relaxed);
        turn.store(other, memory_order_relaxed);
        atomic_thread_fence(memory_order_seq_cst);

        while (interested[other].load(memory_order_relaxed) && turn.load(memory_order_relaxed) == other) { /* spin */ }
    }
    void release(int id)
    {
        interested[id].store(false, memory_order_relaxed);
    }
}