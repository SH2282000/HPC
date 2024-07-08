// Assume that memory management is handled by the caller of the stack and is not the
// responsibility of your implementation. You can use a counted pointer in the following
// form in your implemenation.
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>

std::mutex cout_mutex;

// Node structure for the stack
template <typename T>
struct Node
{
    T data;
    Node *next;
    Node(T value) : data(value), next(nullptr) {}
};

// template <typename T>
// struct CountedPtr
// {
//     Node<T> *ptr = new Node<T>(nullptr);
//     unsigned count = 0;

//     CountedPtr(Node<T> *p = nullptr, unsigned c = 0) : ptr(p), count(c) {}
// };

// Lock-free stack using atomic operations
template <typename T>
class TreiberStack
{
public:
    TreiberStack() : head(nullptr) {}

    void push(T value)
    {
        Node<T> *newNode = new Node<T>(value);
        newNode->next = head.load();

        // Try to update the head to point to the new node
        while (!head.compare_exchange_weak(newNode->next, newNode))
            ;
    }

    bool pop(T &value)
    {
        Node<T> *oldHead = head.load();

        // Try to update the head to the next node
        while (oldHead && !head.compare_exchange_weak(oldHead, oldHead->next))
            ;

        if (oldHead == nullptr)
        {
            return false; // Stack is empty
        }

        value = oldHead->data;
        delete oldHead;
        return true;
    }

    ~TreiberStack()
    {
        while (Node<T> *node = head.load())
        {
            head.store(node->next);
            delete node;
        }
    }

private:
    std::atomic<Node<T> *> head;
};

void push_to_stack(TreiberStack<int> &stack, int value, int delay)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    stack.push(value);
    std::lock_guard<std::mutex> guard(cout_mutex);
    std::cout << "Pushed " << value << " after " << delay << " ms\n";
}

// Example usage
int main(int argc, char **argv)
{
    TreiberStack<int> stack;
    std::vector<std::thread> threads;
    size_t num_tials = 5;

    if (argc == 2)
    {
        num_tials = std::strtol(argv[1], NULL, 10);
    }

    for (size_t i = 0; i < num_tials; i++)
    {
        int delay = std::rand() % 100;
        threads.emplace_back(push_to_stack, std::ref(stack), i + 1, delay);
    }

    // Join all threads
    for (auto &t : threads)
    {
        t.join();
    }

    int value;
    while (stack.pop(value))
    {
        std::cout << value << std::endl;
    }

    return 0;
}
