#include <atomic>
#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <cassert>

using namespace std;

struct QNode
{
    atomic<QNode *> next;
    atomic<bool> wait;

    QNode() : next(nullptr), wait(true) {}
};

class MCSLock
{
public:
    MCSLock() : tail(nullptr) {}

    void acquire(QNode *p)
    {
        p->next.store(nullptr, memory_order_relaxed);
        p->wait.store(true, memory_order_relaxed);
        QNode *prev = tail.exchange(p, memory_order_acq_rel);
        if (prev != nullptr)
        {
            prev->next.store(p, memory_order_release);
            while (p->wait.load(memory_order_acquire)) { /* spin */}
        }
    }

    void release(QNode *p)
    {
        QNode *succ = p->next.load(memory_order_acquire);
        if (succ == nullptr)
        {
            QNode *expected = p;
            if (tail.compare_exchange_strong(expected, nullptr, memory_order_acq_rel))
            {
                return;
            }
            // Wait until the successor is set
            while ((succ = p->next.load(memory_order_acquire)) == nullptr){/*spin*/}
        }
        succ->wait.store(false, memory_order_release);
    }

private:
    atomic<QNode *> tail;
};

MCSLock lock;
int shared_counter = 0;
void increment_counter(int id, int increments)
{
    QNode node;
    for (int i = 0; i < increments; ++i)
    {
        lock.acquire(&node);
        ++shared_counter;
        lock.release(&node);
    }
}

void contention_test(int id, int operations)
{
    QNode node;
    for (int i = 0; i < operations; ++i)
    {
        lock.acquire(&node);
        // Critical section
        this_thread::sleep_for(chrono::microseconds(10));
        lock.release(&node);
        // Non-critical section
        this_thread::sleep_for(chrono::microseconds(10));
    }
}

// Fairness test: ensure order of lock acquisition
atomic<int> order_counter(0);
vector<int> order_of_acquisition;

void fairness_test(int id)
{
    QNode node;
    for (int i = 0; i < 10; ++i)
    {
        lock.acquire(&node);
        order_of_acquisition.push_back(order_counter++);
        lock.release(&node);
    }
}

void run_tests()
{
    const int num_threads = 10;
    const int increments = 1000;

    shared_counter = 0;
    order_counter.store(0);
    order_of_acquisition.clear();

    // Basic test
    {
        vector<thread> threads;
        for (int i = 0; i < num_threads; ++i)
        {
            threads.emplace_back(increment_counter, i, increments);
        }
        for (auto &t : threads)
        {
            t.join();
        }
        assert(shared_counter == num_threads * increments);
        cout << "Basic test passed. Shared counter: " << shared_counter << endl;
    }

    // Contention test
    {
        vector<thread> threads;
        for (int i = 0; i < num_threads; ++i)
        {
            threads.emplace_back(contention_test, i, increments);
        }
        for (auto &t : threads)
        {
            t.join();
        }
        cout << "Contention test completed." << endl;
    }

    // Fairness test
    {
        vector<thread> threads;
        for (int i = 0; i < num_threads; ++i)
        {
            threads.emplace_back(fairness_test, i);
        }
        for (auto &t : threads)
        {
            t.join();
        }
        bool ordered = true;
        for (size_t i = 1; i < order_of_acquisition.size(); ++i)
        {
            if (order_of_acquisition[i - 1] > order_of_acquisition[i])
            {
                ordered = false;
                break;
            }
        }
        assert(ordered);
        cout << "Fairness test passed. Order of acquisition is correct." << endl;
    }
}

int main()
{
    run_tests();
    return 0;
}
