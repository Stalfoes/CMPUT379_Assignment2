#include <cstdio>	// perror, fgets, printf
#include <cstdlib>	// atoi
#include <cctype>	// isdigit
#include <string>
#include <queue>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

/* Declare the external C file's functions */
extern "C" {
	void Trans(int n);
	void Sleep(int n);
};

using namespace std;

atomic<bool> * asking_for_work;	// Atomic objects used by the producer to see if everything is done
								// Used to make sure a safe exit occurs
mutex printing_mutex;			// Used to control who is printing and who is waiting to print
mutex work_queue_mutex;			// Used to control who is accessing the queue for work
queue<int> *work_queue;			// The queue for the work

void thread_method(int id) {
	printing_mutex.lock();
	printf("Thread %d says hello.\n", id);
	printing_mutex.unlock();
}

int main(int argc, char *argv[]) {

	/* GET PARAMETERS */

	if (argc < 2) {
		perror("prodcon requires at least 1 argument. prodcon nthreads <id>\n");
		return 1;
	}

	const int nthreads = atoi(argv[1]);
	string id = "0";

	if (argc >= 3) {
		if ( isdigit( argv[2][0] ) ) {
			id = argv[2];
		}
	}

	// printf("You entered details NTHREADS = %d, ID = %s\n", nthreads, id.c_str());

	/* START OF PRODCON */
	/* SPAWN THE CONSUMER THREADS AND SAVE THEIR STATES */

	asking_for_work = new atomic<bool>[nthreads];

	for (int i = 0; i < nthreads; i++) {
		asking_for_work[i].store(false, memory_order_release);
	}

	thread consumers[nthreads];

	for (int i = 0; i < nthreads; i++) {
		consumers[i] = thread(thread_method, i);
	}

	for (int i = 0; i < nthreads; i++) {
		consumers[i].join();
	}

	printf("The consumers have finished all their work.\n");

	return 0;
}
