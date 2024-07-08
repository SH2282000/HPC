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

    #pragma omp parallel num_threads(7)
    {
        int thread_id = omp_get_thread_num();
        std::string value = "Thread " + std::to_string(thread_id);

        // Propose a value and get the consensus value
        std::string consensus_value = consensus.propose(value); // <---- first value proposed from all threads will be returned

        // Store the result in the corresponding array element
        results[thread_id] = consensus_value;
    }

    // Output the consensus values from both threads
    for (size_t i = 0; i < results->length(); i++)
    {
        std::cout << "Thread 0 consensus value: " << results[i] << std::endl;
    }
    
    return 0;
}