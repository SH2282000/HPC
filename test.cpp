#include <atomic>
#include <iostream>

using namespace std;

class Banana {
	static int lenght;
};

int atomic_incr(std::atomic<int>& val, int incr) { // GOAL: conserve atomicity
	int oldval;
	int newval;

	while (!val.compare_exchange_strong(oldval, newval)) {
		// oldval updated from compare_exchange_strong()
		newval = oldval + incr;
	}
	return newval;
}

int main() {
	atomic<int> a, b;
	b.store(42);
	a.store(b.load());

	vector< atomic<int> > vec; // make a vecor of atomic objects (atoms?)

	Banana banana;

	cout << "hello " << atomic_incr(a, 2) << endl;
	while(1);
}