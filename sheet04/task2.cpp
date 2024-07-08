#include <chrono >
#include <thread >
#include "lock.h"
using namespace std::chrono;
using namespace std;
/* Configurations */
#define NUM_OF_THREADS 10
#define NUM_OF_ITERS 1000
#define OUTSIDE_WORK 3
#define N 10
/* Shared variables */
lock_mcs lock; // for example: MCS Lock
/* Private variables */
// thread_local int dummy;
/* Child thread 's code */
void thread_entry(...)
{
    for (auto i = 0; i < NUM_OF_ITERS; ++i)
    {
        lock.acquire();
        // check if a number N is a prime
        is_prime(N);
        lock.release();
        for (auto j = 0; j < OUTSIDE_WORK; ++j)
            is_prime(N);
    }
}

// is_prime() is meant to represent work done inside the critical section and implements a simple primality test.
int is_prime(int p)
{
    int d;
    for (d = 2; d < p; d = d + 1)
        if (p % d == 0)
            return 0;
    return 1;
}

int main(int argc, char const *argv[])
{
    vector<thread> threads;
    return 0;
}
