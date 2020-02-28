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

#include "tands.h"
#include "buffer.h"	// Buffer

using namespace std;

#define MAX_LINE_LENGTH 10

clock_t program_start;

Buffer buffer;

mutex printing_mutex;			// Used to control who is printing and who is waiting to print
int nthreads;

void thread_method(int id) {

	while (true) {

		// ASK

		float dt = ((float)(clock() - program_start)) / CLOCKS_PER_SEC;	// Calculate the time we're at
		printing_mutex.lock();				// PRINT THAT WE'RE ASKING FOR WORK
		printf("%.3f ID= %d      Ask\n", dt, id);
		printing_mutex.unlock();

		// RECEIVE

		int work, nq;
		buffer.pop(work, nq);
		if (work == NO_MORE_WORK) break;

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

	buffer.resize(2 * nthreads);
	// buffer.push(100); buffer.push(2); buffer.push(3); buffer.push(4); buffer.push(5);
	// buffer.push(NO_MORE_WORK);

	program_start = clock();

	thread consumers[nthreads];

	for (int i = 0; i < nthreads; i++) {
		consumers[i] = thread(thread_method, i + 1);
	}

	// Get input from stdin
	string line;
	char input[MAX_LINE_LENGTH];
	// char c[1] = '\0';
	while (fgets(input, MAX_LINE_LENGTH, stdin) != NULL) {
		
		/*while (strcmp(c, 1, '\n') == false) {
			line.append(c);
		}*/
		line = input;
		
		string amount_s = line.substr(1, line.length() - 1);
		int amount = stoi(amount_s.c_str());
		string work_type = line.substr(0, 1);

		printing_mutex.lock();
		printf("PRODUCER - Type: %s, Amount: %s -> %d\n", work_type.c_str(), amount_s.c_str(), amount);
		printing_mutex.unlock();

	}

	/*
	int amount = stoi(line.substr(1));
	string work_type = line.substr(0, 1);

	if (strcmp(work_type, "T")) {
		
	} else if (strcmp(work_type)	
	*/

	buffer.push(NO_MORE_WORK);
	for (int i = 0; i < nthreads; i++) {
		consumers[i].join();
	}

	printf("PROGRAM END\n");

	return 0;
}
