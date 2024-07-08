

**Mutual exclusion**: Execution of the *Critical Section (CS)* by only 1 thread at a time.



- **Deadlock freedom**: thread(s) try to acquire lock then some threads cannot enter the *Critical Section*, only 1 thread can access the *CS* the other ones are *starving*
- **Starvation freedom**: Not going to starve forever (eventually succeeds)
- **Fairness**: 2 threads try to acquire lock, but A try before B then A enter *Critical Section* before

