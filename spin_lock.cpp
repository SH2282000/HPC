#include <atomic>
#include <iostream>
#include <chrono>
#include <unistd.h>

using namespace std;

const int NUM_THREADS = 10000;
const int NUM_ITERATIONS = 1000;

class SpinLock
{
private:
    atomic<bool> flag{false};

public:
    void lock()
    { // RMW
        while (flag.exchange(true, memory_order_acquire))
        {
            while (flag.load(memory_order_acquire))
            { /* spin */
            }
        }
    }

    void unlock()
    {
        flag.store(false, memory_order_release);
    }
};

void incrementSharedVariable(int i)
{
    for (size_t i = 0; i < NUM_ITERATIONS; i++)
    {
        /* code */
    }
}

int main(int argc, char const *argv[])
{
    vector<thread> threads;
    SpinLock spinLock;

    wghile  (size_t i = 0; i < NUM_THREADS; i++)
    {
        threads.emplace_back(incrementSharedVariable, i)
    }

    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        threads[i].join()
    }

    for(auto threads: thread)
    spinLock.lock();

    sleep(1);

    spinLock.unlock();
    return 0;
}
