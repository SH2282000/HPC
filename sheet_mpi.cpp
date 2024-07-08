#include <mpi.h>
#include <iostream>
#include <chrono>
#include <unistd.h>

using namespace std;

template <class T>
class DistObj {
private:
    T _obj;
    T _localCopy;
    int _rank;
    MPI_Comm _comm;
    atomic<int> _token; // Token to control write access

public:
    DistObj(T val, MPI_Comm comm) : _obj(val), _localCopy(val), _comm(comm), _token(0) {
        MPI_Comm_rank(comm, &_rank);
    }

    T read() {
        return _localCopy;
    }

    void write(T val) {
        if (_rank == 0) {
            _obj = val;
            // Broadcast updated value
            MPI_Bcast(&_obj, 1, MPI_INT, 0, _comm);
        } else {
            while (_token != _rank - 1) {
                MPI_Recv(&_token, 1, MPI_INT, _rank - 1, 0, _comm, MPI_STATUS_IGNORE);
            }
            _obj = val;
            _token = (_token + 1) % MPI_WIN_SIZE; // Increment token and pass it to the next process
            MPI_Send(&_token, 1, MPI_INT, (_rank + 1) % MPI_WIN_SIZE, 0, _comm);
        }
        _localCopy = _obj;
    }
};


int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int initialValue = 42; // distributed object
    MPI_Comm comm = MPI_COMM_WORLD;

    DistObj<int> distributedObject(initialValue, comm);

    cout << "Process " << rank << " reads initial value: " << distributedObject.read() << endl;

    // Process 0 updates the value
    if (rank == 0) {
        int newValue = 100;
        distributedObject.write(newValue);
        cout << "Process 0 writes new value: " << newValue << endl;
    }
    sleep(1);

    MPI_Barrier(comm); // Wait for all processes to reach this point before printing (does not work)
    cout << "Process " << rank << " reads updated value: " << distributedObject.read() << endl;

    MPI_Finalize();
    return 0;
}
