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
#include <vector>

#include "tands.h"
#include "buffer.h"	// Buffer

using namespace std;

#define MAX_LINE_LENGTH 10

chrono::milliseconds program_start;

Buffer buffer;

mutex printing_mutex;			// Used to control who is printing and who is waiting to print
int nthreads;

/* atomic objects to store summary information */
int nWork = 0, nAsk = 0, nReceive = 0, nComplete = 0, nSleep = 0;
vector<int> workTaken;

//mutex tttMutex;		// for atomic totalTimeTaken. Can't use atomic<float> as we need to do more than one operation at a time
//float totalTimeTaken;

float dTime() {
	chrono::milliseconds dt_ms = chrono::duration_cast<chrono::milliseconds> ( 
		chrono::system_clock::now().time_since_epoch()
	) - program_start;	// Calculate the time we're at
	return ((float) dt_ms.count()) / 1000;
}

void thread_method(int id) {

	while (true) {

		// ASK
		
		printing_mutex.lock();
		printf("%8.3f ID= %d      Ask\n", dTime(), id);
		printing_mutex.unlock();
		nAsk++;

		// The last time in execution will always occur after an ask command on the consumer side
		// Only other possibility is a producer waking from a sleep
/*		tttMutex.lock();
		if (dt > totalTimeTaken) totalTimeTaken = dt;
		tttMutex.unlock();
*/
		// RECEIVE

		int work, nq;
		buffer.pop(work, nq);
		if (work == NO_MORE_WORK) break;

		printing_mutex.lock();
		printf("%8.3f ID= %d Q= %d Receive %5d\n", dTime(), id, nq, work);
		printing_mutex.unlock();
		nReceive++;
		workTaken[id - 1]++;		

		// WORK

		Trans(work);

		// COMPLETE

		printing_mutex.lock();
		printf("%8.3f ID= %d      Complete %4d\n", dTime(), id, work);
		printing_mutex.unlock();
		nComplete++;

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

	workTaken.resize(nthreads);

	buffer.resize(2 * nthreads);
	// buffer.push(100); buffer.push(2); buffer.push(3); buffer.push(4); buffer.push(5);
	// buffer.push(NO_MORE_WORK);

	program_start = clock();

	thread consumers[nthreads];

	for (int i = 0; i < nthreads; i++) {
		workTaken[i] = 0;
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

		if (work_type.compare("T") == 0) {

			buffer.push(amount, nq);
	
			printing_mutex.lock();
			printf("%8.3f ID= 0 Q= %d Work %8d\n", dTime(), nq, amount);
			printing_mutex.unlock();
			nWork++;

		} else {

			printing_mutex.lock();
			printf("%8.3f ID= 0      Sleep %7d\n", dTime(), amount);
			printing_mutex.unlock();

			Sleep(amount);

			nSleep++;
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

	// Producer uses the total time as the time when all the threads have shut down
	// Causes a slight descepancy between what is printed and what is calculated
	float finalTime = dTime();

	printf("Summary:\n");
	printf("    Work %9d\n", nWork);
	printf("    Ask %10d\n", nAsk);
	printf("    Receive %6d\n", nReceive);
	printf("    Complete %5d\n", nComplete);
	printf("    Sleep %8d\n", nSleep);
	
	for (int i = 0; i < nthreads; i++) {
		printf("    Thread %2d %4d\n", i+1, workTaken[i]);
	}

//	printf("Final time= %f\n", finalTime);
	printf("Transactions per second: %.2f\n", nComplete / finalTime);

	// printf("PROGRAM END\n");

	return 0;
}
