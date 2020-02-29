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
		
		printing_mutex.lock();
		float dt = ((float)(clock() - program_start)) / CLOCKS_PER_SEC;	// Calculate the time we're at
		printf("%.3f ID= %d      Ask\n", dt, id);
		printing_mutex.unlock();

		// RECEIVE

		int work, nq;
		buffer.pop(work, nq);
		if (work == NO_MORE_WORK) break;

		printing_mutex.lock();
		dt = ((float)(clock() - program_start)) / CLOCKS_PER_SEC;
		printf("%.3f ID= %d Q= %d Receive     %d\n", dt, id, nq, work);
		printing_mutex.unlock();

		// WORK

		Trans(work);

		// COMPLETE

		printing_mutex.lock();
		dt = ((float)(clock() - program_start)) / CLOCKS_PER_SEC;
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
	int nq;
	// char c[1] = '\0';
	while (fgets(input, MAX_LINE_LENGTH, stdin) != NULL) {
				
		line = input;
		
		// string amount_s = line.substr(1);
		int amount = stoi(line.substr(1));
		string work_type = line.substr(0, 1);

		float dt;		

		if (work_type.compare("T") == 0) {

			buffer.push(amount, nq);
	
			printing_mutex.lock();
			dt = ((float)(clock() - program_start)) / CLOCKS_PER_SEC;
			printf("%.3f ID= 0 Q= %d Work        %d\n", dt, nq, amount);
			printing_mutex.unlock();
		} else {
			printing_mutex.lock();
			dt = ((float)(clock() - program_start)) / CLOCKS_PER_SEC;
			printf("%.3f ID= 0      Sleep       %d\n", dt, amount);
			printing_mutex.unlock();

			Sleep(amount);
		}
		//buffer.push(amount, nq);

		/*printing_mutex.lock();
		printf("PRODUCER - Type: %s -> %s, Amount: %d\n", work_type.c_str(), type.c_str(), amount);
		printing_mutex.unlock();*/

	}


	/*
	int amount = stoi(line.substr(1));
	string work_type = line.substr(0, 1);

	if (strcmp(work_type, "T")) {
		
	} else if (strcmp(work_type)	
	*/

	buffer.push(NO_MORE_WORK, nq);
	for (int i = 0; i < nthreads; i++) {
		consumers[i].join();
	}

	// printf("PROGRAM END\n");

	return 0;
}
