# Sheet 3

## Peterson Lock

> What happens if we implement the lock more selshly with `turn = self`; instead?

Both threads will try to access the critical section at the same time and not let each other get this access. This will result in a race condition and/or the starvation of one thread.
Thus, the mutual exclusion and fairness are compromised.

> Does this algorithm still guarantee mututal exclusion? Is the lock still starvation-free?

Yes, the mutual exclusion and fairness is guaranteed for 2 threads. Thus, it is starvation free. However this behavior cannot be guaranteed for more than 2 threads.

> Implement the lock using C++ atomics

Here's how we can implement the perterson algorithm with atomics:

```cpp
class Peterson {
    private:
        atomic<bool> interested[2];
        atomic<int> turn;

    public:
        Peterson() {
            interested[0].store(0)
            interested[1].store(0)
            turn.store(0)
        }

    void acquire(int id) {
        int other = 1 - id
        interested[id].store(true, memory_order_relaxed)
        turn.store(other, memory_order_relaxed)
        atomic_thread_fence(memory_order_seq_cst)

        while(interested[other].load(memory_order_relaxed) && turn.load(memory_order_relaxed) == other) { /* spin */ }
    }
    void release(int id) {
        interested[id].store(false, memory_order_relaxed)
    }
}
```

## MCS Lock

> Implement the lock using C++ atomics. 

```cpp
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
```

> Devise a method for testing your implementation and thoroughly test your lock implementation in various scenarios

I implemented 3 types of tests:

- Basic test (just proceed to the acquisition by index order)
- Contention test
- Fairness test


```cpp
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
```