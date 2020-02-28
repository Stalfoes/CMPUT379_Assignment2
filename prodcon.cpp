#include <cstdio>	// perror, fgets, printf
#include <cstdlib>	// atoi
#include <cctype>	// isdigit
#include <string>
#include <queue>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <ctime>
#include <condition_variable>

#include "tands.h"

using namespace std;

#define NO_MORE_WORK -1

clock_t program_start;

/* condition_variable 's used to control the mutex's such that they wait for a condition */
condition_variable queue_not_full_cv;
condition_variable queue_not_empty_cv;

mutex printing_mutex;			// Used to control who is printing and who is waiting to print
mutex work_queue_mutex;			// Used to control who is accessing the queue for work
queue<int> work_queue;			// The queue for the work

int nthreads;

bool queue_is_not_full() {
	return work_queue.size() < (2 * nthreads);
}

bool queue_is_not_empty() {
	return work_queue.size() > 0;
}

void thread_method(int id) {

	while (true) {

		// ASK

		float dt = ((float)(clock() - program_start)) / CLOCKS_PER_SEC;	// Calculate the time we're at
		printing_mutex.lock();				// PRINT THAT WE'RE ASKING FOR WORK
		printf("%.3f ID= %d      Ask\n", dt, id);
		printing_mutex.unlock();

		// RECEIVE

		unique_lock<mutex> lk(work_queue_mutex);
		queue_not_empty_cv.wait(lk, queue_is_not_empty);			// ACCESS THE QUEUE
		int work = work_queue.front();
		if (work == NO_MORE_WORK) { 		// Leave the thread if we've been instructed to quit
			work_queue_mutex.unlock();
			break;
		}
		work_queue.pop();
		const int nq = work_queue.size();
		queue_not_full_cv.notify_one();			// Notify that the queue is no longer full to the producer
		work_queue_mutex.unlock();

		dt = ((float)(clock() - program_start)) / CLOCKS_PER_SEC;
		printing_mutex.lock();				// PRINT THAT WE RECEIVED WORK
		printf("%.3f ID= %d Q= %d Receive     %d\n", dt, id, nq, work);
		printing_mutex.unlock();

		// WORK

		Trans(work);

		// COMPLETE

		dt = ((float)(clock() - program_start)) / CLOCKS_PER_SEC;
		printing_mutex.lock();				// PRINT THAT WE COMPLETED WORK
		printf("%.3f ID= %d      Complete    %d\n", dt, id, work);
		printing_mutex.unlock();

		break;

	}
}

int main(int argc, char *argv[]) {

	/* GET PARAMETERS */

	if (argc < 2) {
		perror("prodcon requires at least 1 argument. prodcon nthreads <id>\n");
		return 1;
	}

	nthreads = atoi(argv[1]);
	string id = "0";

	if (argc >= 3) {
		if ( isdigit( argv[2][0] ) ) {
			id = argv[2];
		}
	}

	// printf("You entered details NTHREADS = %d, ID = %s\n", nthreads, id.c_str());

	/* START OF PRODCON */
	/* SPAWN THE CONSUMER THREADS AND SAVE THEIR STATES */

	work_queue.push(100); work_queue.push(2); work_queue.push(3); work_queue.push(4); work_queue.push(5);

	program_start = clock();

	thread consumers[nthreads];

	for (int i = 0; i < nthreads; i++) {
		consumers[i] = thread(thread_method, i + 1);
	}

	for (int i = 0; i < nthreads; i++) {
		consumers[i].join();
	}

	printf("PROGRAM END\n");

	return 0;
}
