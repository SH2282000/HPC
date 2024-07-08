# Worksheet 7

- [Worksheet 7](#worksheet-7)
  - [1. Wait-free Consensus Objects](#1-wait-free-consensus-objects)
  - [2. Load-Linked/Store-Conditional](#2-load-linkedstore-conditional)


## 1. Wait-free Consensus Objects
- a)  Show that a wait-free stack can be used to implement a consensus object that achieves
**wait-free consensus** for two threads.

The stack operations (push and pop) are wait-free, ensuring that both threads can complete their operations in a finite number of steps.
```cpp
#include <iostream>
#include <stack>
#include <atomic>
#include <omp.h>

template<typename T>
class WaitFreeStack {
private:
    std::stack<T> stack;
    std::atomic_flag lock = ATOMIC_FLAG_INIT;  // Spinlock for atomic operations

public:
    void push(T value) {
        while (lock.test_and_set(std::memory_order_acquire)) { }  // Acquire the lock
        stack.push(value);
        lock.clear(std::memory_order_release);  // Release the lock
    }

    T pop() {
        while (lock.test_and_set(std::memory_order_acquire)) { }  // Acquire the lock
        if (stack.empty()) {
            lock.clear(std::memory_order_release);  // Release the lock if empty
            return T();  // Return default value if empty
        }
        T value = stack.top();
        stack.pop();
        lock.clear(std::memory_order_release);  // Release the lock
        return value;
    }
};
```
let's implement the consensus object using this wait-free stack:
```cpp
template<typename T>
class ConsensusObject {
private:
    WaitFreeStack<T> stack;

public:
    T propose(T value) {
        stack.push(value);
        T consensus_value = stack.pop();
        return consensus_value;
    }
};

```

- The **stack operations (push and pop) are wait-free**, ensuring that both threads can complete their operations in a finite number of steps.
- Due to the atomic nature of stack operations, if two threads push their values concurrently, one of the threads **will pop the value that was last pushed**, and the other thread will **pop the same or the other value**.
- The key idea is that both threads will *either pop the same value* (if one thread pushes and the other pops immediately) or *each thread will pop the value that the other thread pushed*, which still **results in agreement since they will both end up with one of the proposed values**.

From the lecture:

> Consensus problem
> – N “actors” (threads/processes) need to agree on a value
> – Each actor `i` proposes a value vi
> – After consensus, all actors agree on the same value v, which must be one of
the of proposed values (v  {v1,…,vN})

With the *wait-free stack*:
- **Agreement**: Both threads will return the same value or one of the proposed values due to the atomic nature of the stack operations.
- **Validity**: The value returned is always one of the values proposed by the threads.
- **Termination**: Since the stack operations are wait-free, both threads will complete their operations in a finite number of steps.

> Corollary 5.1.1. Suppose one can implement an object of class C from one or
more objects of class D, together with some number of atomic registers. If class
C solves n-consensus, then so does class D.

- b)  Show that FAA (fetch-and-add) can be used to implement a consensus object that achieves
wait-free consensus for two threads.

**Key idea**: Use an atomic counter to keep track of which thread comes first. Each thread uses FAA to increment the counter and check the result to determine if it was the first or second thread.

Consensus Value Decision: The *first thread's proposed value becomes the consensus value*.

```cpp
#include <iostream>
#include <atomic>
#include <string>
#include <omp.h>

class ConsensusObject {
private:
    std::atomic<int> counter;
    std::string chosenValue;
    std::atomic<bool> valueChosen;

public:
    ConsensusObject() : counter(0), valueChosen(false) {}

    std::string propose(const std::string& value) {
        int pos = counter.fetch_add(1);  // FAA operation
        if (pos == 0) {
            // First thread to increment the counter
            chosenValue = value;
            valueChosen.store(true, std::memory_order_release);
        } else {
            // Spin until the value is chosen by the first thread
            while (!valueChosen.load(std::memory_order_acquire)) { }
        }
        return chosenValue;
    }
};

int main() {
    ConsensusObject consensus;

    std::string results[2];

    #pragma omp parallel num_threads(2)
    {
        int thread_id = omp_get_thread_num();
        std::string value = "Thread " + std::to_string(thread_id);

        // Propose a value and get the consensus value
        std::string consensus_value = consensus.propose(value); // <---- first value proposed from all threads will be returned

        // Store the result in the corresponding array element
        results[thread_id] = consensus_value;
    }

    // Output the consensus values from both threads
    std::cout << "Thread 0 consensus value: " << results[0] << std::endl;
    std::cout << "Thread 1 consensus value: " << results[1] << std::endl;

    return 0;
}
```


Thus, Fetch-and-Add operation can be used to implement a wait-free consensus object for two threads.

- c) The solution presented in the lecture for universal consensus using CAS made use of dedicated initial value (denoted ⊥). Derive a solution that doesn’t require any such dedicated
initial value.

To derive a solution that doesn't require a dedicated initial value for implementing consensus using Compare-And-Swap (CAS), we can slightly modify the approach. Instead of relying on a special initial value, we can utilize **an atomic counter to coordinate the proposals from the threads**.

```cpp
#include <iostream>
#include <atomic>
#include <string>
#include <omp.h>

class ConsensusObject {
private:
    std::atomic<int> flag;
    std::string consensusValue;
    std::atomic<bool> isSet;

public:
    ConsensusObject() : flag(0), isSet(false) {}

    std::string decide(const std::string& value) {
        int expected = 0;
        // Use CAS to attempt setting the consensus value
        if (flag.compare_exchange_strong(expected, 1)) {
            // This thread successfully sets the consensus value
            consensusValue = value;
            isSet.store(true, std::memory_order_release);
        } else {
            // Spin until the value is set by the first thread
            while (!isSet.load(std::memory_order_acquire)) {}
        }
        return consensusValue;
    }
};

int main() {
    ConsensusObject consensus;

    std::string results[2];

    #pragma omp parallel num_threads(2)
    {
        int thread_id = omp_get_thread_num();
        std::string value = "Thread " + std::to_string(thread_id);

        // Propose a value and get the consensus value
        std::string consensus_value = consensus.decide(value);

        // Store the result in the corresponding array element
        results[thread_id] = consensus_value;
    }

    // Output the consensus values from both threads
    std::cout << "Thread 0 consensus value: " << results[0] << std::endl;
    std::cout << "Thread 1 consensus value: " << results[1] << std::endl;

    return 0;
}

```

output:

```shell
❯ ./consensus_cas
Thread 0 consensus value: Thread 0
Thread 1 consensus value: Thread 0
```


Thus, we proved the following:

- **Agreement**: Both threads will *agree on the value set by the first thread* to successfully perform the CAS operation.
- **Validity**: The value returned is *always one of the proposed values*.
- **Termination**: Since CAS is wait-free, both threads will complete their operations in a *finite number* of steps.

- d) Derive a wait-free consensus object based on **atomic memory-memory swap**
and atomic read-write registers that achieves wait-free consensus for an arbitrary number
of threads.

The `swap(a, b)` operation will be used to coordinate and achieve consensus among all threads:

**Phase 1: Proposal Stage**

Each *thread $i$ attempts to write its proposed value $v_i$ into R[i]* using atomic writes.

**Phase 2: Coordination and Decision Stage**

Use atomic *memory-memory swap operations to coordinate and decide on a final consensus value*.

Pseudo-code:
```txt
repeat:
  tmp := swap(R[0], R[i])
  if tmp == ⊥:    // ⊥ indicates uninitialized or no decision made
    R[i] := v_i   // Write proposed value to my register
    swap(R[i], R[0])
    decide := v_i // Set decide to my proposed value

    for j := 1 to N:
      swap(R[0], R[j])

    return decide
  else:
    swap(R[i], R[0])
```

- d)

Here are the outputs for all consensus ojects with multiple threads threads (source codes are in the ZIP file):

```shell
❯ ./consensus_faa
Thread 0 consensus value: Thread 0
Thread 0 consensus value: Thread 0
Thread 0 consensus value: Thread 0
Thread 0 consensus value: Thread 0
Thread 0 consensus value: Thread 0
Thread 0 consensus value: Thread 0
Thread 0 consensus value: Thread 0
Thread 0 consensus value: Thread 0
...
```

## 2. Load-Linked/Store-Conditional

**Weak CAS**:
```cpp
bool weakCAS(word *a, word expected, word new_value) {
    word current_value = LL(a); // Perform Load-Linked

    if (current_value == expected) {
        if (SC(a, new_value)) {
            // SC succeeded (memory location was not changed between LL and SC)
            return true;
        } else {
            // SC failed (memory location was changed between LL and SC)
            return false;
        }
    } else {
        // CAS failed because current_value != expected
        return false;
    }
}
```
Attempt to update a memory location only if its current value matches the expected value. If the memory location has changed since it was last read (LL), the CAS operation fails.



**Strong CAS**:
```cpp
bool strongCAS(word *a, word expected, word new_value) {
    bool success = false;
    do {
        word current_value = LL(a);
        if (current_value == expected) {
            success = SC(a, new_value); // Attempt Store-Conditional
        } else {
            // CAS failed because current_value != expected
            success = false;
        }
    } while (!success);

    return true;  // <-- can only return true
}
```

CAS operation **succeeds only if the memory location a has not been modified since the LL operation**.


