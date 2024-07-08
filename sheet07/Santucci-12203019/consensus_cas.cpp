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

    #pragma omp parallel num_threads(10)
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