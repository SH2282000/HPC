# Worksheet 8

- [Worksheet 8](#worksheet-8)
  - [1. Treiber’s Stack](#1-treibers-stack)


## 1. Treiber’s Stack
- a) Implement the Treiber stack in C++ with counted pointers to avoid the ABA problem.

Treiber lock-free stack algorithm and presented counted pointers is a way to avoid the ABA problem:

```cpp
class Node {
    T val;
    Node *next;
}
class Stack{
    atomic Node *top=NULL;
    Node* pop();
    void push(Node*);
}
```

