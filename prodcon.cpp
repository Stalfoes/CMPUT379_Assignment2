#include <cstdio>	// perror, fgets, printf
#include <cstdlib>	// atoi
#include <cctype>	// isdigit
#include <string>
#include <queue>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <vector>

#include "tands.h"
#include "buffer.h"	// Buffer

using namespace std;

#define MAX_INPUT_LINE_LENGTH 10
#define MAX_OUTPUT_LINE_LENGTH 50

chrono::milliseconds program_start;

Buffer buffer;

mutex printing_mutex;			// Used to control who is printing and who is waiting to print
int nthreads;

/* Summary information */
int nWork = 0, nAsk = 0, nReceive = 0, nComplete = 0, nSleep = 0;
vector<int> workTaken;

//mutex tttMutex;		// for atomic totalTimeTaken. Can't use atomic<float> as we need to do more than one operation at a time
//float totalTimeTaken;

/* Used to output to the necessary file */
FILE* outputFile;
char output_str[MAX_OUTPUT_LINE_LENGTH] = "\0";
int n_characters_written;

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
		n_characters_written = sprintf(output_str, "%8.3f ID= %d      Ask\n", dTime(), id);
		fputs(output_str, outputFile);
		printing_mutex.unlock();
		nAsk++;

		// RECEIVE

		int work, nq;
		buffer.pop(work, nq);
		if (work == NO_MORE_WORK) break;

		printing_mutex.lock();
		n_characters_written = sprintf(output_str, "%8.3f ID= %d Q= %d Receive %5d\n", dTime(), id, nq, work);
		fputs(output_str, outputFile);
		printing_mutex.unlock();
		nReceive++;
		workTaken[id - 1]++;		

		// WORK

		Trans(work);

		// COMPLETE

		printing_mutex.lock();
		n_characters_written = sprintf(output_str, "%8.3f ID= %d      Complete %4d\n", dTime(), id, work);
		fputs(output_str, outputFile);
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

	/* SET UP LOG FILE */

	string output_file_name = "prodcon.";
	if (id.compare("0") != 0) {
		output_file_name += id + ".";
	}
	output_file_name += "log";
	// printf("Output file: %s", output_file_name.c_str());
	outputFile = fopen(output_file_name.c_str(), "w");

	// printf("You entered details NTHREADS = %d, ID = %s\n", nthreads, id.c_str());

	/* START OF PRODCON */
	/* SPAWN THE CONSUMER THREADS AND SAVE THEIR STATES */

	workTaken.resize(nthreads);

	buffer.resize(2 * nthreads);

	program_start = chrono::duration_cast<chrono::milliseconds> (
		chrono::system_clock::now().time_since_epoch()
	);

	thread consumers[nthreads];

	for (int i = 0; i < nthreads; i++) {
		workTaken[i] = 0;
		consumers[i] = thread(thread_method, i + 1);
	}

	// Get input from stdin
	string line;
	char input[MAX_INPUT_LINE_LENGTH];
	int nq;

	while (fgets(input, MAX_INPUT_LINE_LENGTH, stdin) != NULL) {
				
		line = input;
		
		// string amount_s = line.substr(1);
		int amount = stoi(line.substr(1));
		string work_type = line.substr(0, 1);	

		if (work_type.compare("T") == 0) {

			buffer.push(amount, nq);
	
			printing_mutex.lock();
			n_characters_written = sprintf(output_str, "%8.3f ID= 0 Q= %d Work %8d\n", dTime(), nq, amount);
			fputs(output_str, outputFile);
			printing_mutex.unlock();
			nWork++;

		} else {

			printing_mutex.lock();
			n_characters_written = sprintf(output_str, "%8.3f ID= 0      Sleep %7d\n", dTime(), amount);
			fputs(output_str, outputFile);
			printing_mutex.unlock();

			Sleep(amount);

			nSleep++;
		}

	}

	buffer.push(NO_MORE_WORK, nq);
	for (int i = 0; i < nthreads; i++) {
		consumers[i].join();
	}

	// Producer uses the total time as the time when all the threads have shut down
	// Causes a slight descepancy between what is printed and what is calculated
	float finalTime = dTime();

	n_characters_written = sprintf(output_str, "Summary:\n    Work %9d\n", nWork);
	fputs(output_str, outputFile);

	n_characters_written = sprintf(output_str, "    Ask %10d\n    Receive %6d\n", nAsk, nReceive);
	fputs(output_str, outputFile);

	n_characters_written = sprintf(output_str, "    Complete %5d\n    Sleep %8d\n", nComplete, nSleep);
	fputs(output_str, outputFile);
	
	for (int i = 0; i < nthreads; i++) {
		n_characters_written = sprintf(output_str, "    Thread %2d %4d\n", i+1, workTaken[i]);
		fputs(output_str, outputFile);
	}

	n_characters_written = sprintf(output_str, "Transactions per second: %.2f\n", nComplete / finalTime);
	fputs(output_str, outputFile);

	fclose(outputFile);

	return 0;
}
