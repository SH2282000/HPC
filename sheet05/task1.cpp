#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <omp.h>

#define EPOCHS 100
#define NTHREADS 4

class Barrier
{
public:
    Barrier(int N) : N(N), count(0), sense(false) {}

    void arrive_and_wait(int tid)
    {
        std::atomic_thread_fence(std::memory_order_release);
        bool lsense = !sense.load();
        if (count.fetch_add(1) == N - 1)
        {
            count.store(0);
            sense.store(lsense);
        }
        else
        {
            while (sense.load() != lsense); // spinning
        }
        std::atomic_thread_fence(std::memory_order_acquire);
    }

private:
    const int N; // Number of threads
    std::atomic<int> count;
    std::atomic<bool> sense;
};

int main(int argc, char *argv[])
{
    Barrier barr(NTHREADS);
#pragma omp parallel num_threads(NTHREADS)
    {
        for (size_t epoch = 0; epoch < EPOCHS; ++epoch)
        {
            std::osyncstream(std::cout) << "Thread " << omp_get_thread_num() << " completed epoch " << epoch << std::endl;
            barr.arrive_and_wait(omp_get_thread_num());

            // simulate computation
        }
    }
    return 0;
}
