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

#define MAX_INPUT_LINE_LENGTH 10	// Max length of work line in input (e.g. T5 or S9)
#define MAX_OUTPUT_LINE_LENGTH 50	// Max length of output line, chosen as 50 for safe number

chrono::milliseconds program_start;	// The start of the program in milliseconds, real time

Buffer buffer;

mutex printing_mutex;				// Used to control who is printing and who is waiting to print, ensures no 'garbled' output

/* Summary information */
int nWork = 0, nAsk = 0, nReceive = 0, nComplete = 0, nSleep = 0;
vector<int> workTaken;				// Number of times each thread has taken work

/* Used to output to the necessary file */
FILE* outputFile;
char output_str[MAX_OUTPUT_LINE_LENGTH] = "\0";	// The string to output, defined global as only ever outputting at one time
int n_characters_written;						// Throw-away variable

// Returns the amount of real time elapsed since the program_start, in seconds
float dTime() {
	chrono::milliseconds dt_ms = chrono::duration_cast<chrono::milliseconds> ( 
		chrono::system_clock::now().time_since_epoch()
	) - program_start;											// Get the real time, and subtract the program start
	return ((float) dt_ms.count()) / 1000;						// Convert to seconds
}

// The main consumer method. All consumers are run in this method
void thread_method(int id) {

	while (true) {

		// ASK
		
		printing_mutex.lock();			// Lock
		n_characters_written = sprintf(output_str, "%8.3f ID= %d      Ask\n", dTime(), id);	// Format the output line
		fputs(output_str, outputFile);	// Print to file
		printing_mutex.unlock();		// Unlock
		nAsk++;

		// RECEIVE

		int work, nq;						// work = 'n' in work, nq = number of queue elements left = 'Q'
		buffer.pop(work, nq);				// Atomically pop
		if (work == NO_MORE_WORK) break;	// If we're signalled to stop, break the loop and exit

		printing_mutex.lock();			// Lock
		n_characters_written = sprintf(output_str, "%8.3f ID= %d Q= %d Receive %5d\n", dTime(), id, nq, work);
		fputs(output_str, outputFile);
		printing_mutex.unlock();		// Unlock
		nReceive++;
		workTaken[id - 1]++;			// We took work, keep track of it

		// WORK

		Trans(work);					// Complete the transaction

		// COMPLETE

		printing_mutex.lock();			// Lock
		n_characters_written = sprintf(output_str, "%8.3f ID= %d      Complete %4d\n", dTime(), id, work);
		fputs(output_str, outputFile);
		printing_mutex.unlock();		// Unlock
		nComplete++;

		// Loop back to asking for work
	}
}

// Producer method. Spawns the threads, performs IO on the provided file and calculates the summary
int main(int argc, char *argv[]) {

	/* GET PARAMETERS */

	if (argc < 2) {		// If no arguments were provided, throw an error
		perror("prodcon requires at least 1 argument. prodcon nthreads <id>\n");
		return 1;
	}

	int nthreads = atoi(argv[1]);	// Convert the number of threads to an integer
	string id = "0";				// Initialize the log ID to 0

	if (argc >= 3) {					// If we have a second argument
		if ( isdigit( argv[2][0] ) ) {	// If it's a digit
			id = argv[2];				// Store that as the ID
		}
	}

	/* SET UP LOG FILE */

	string output_file_name = "prodcon.";	// The log file name
	if (id.compare("0") != 0) {				// If the ID is not zero...
		output_file_name += id + ".";		//		add the ID into the log name
	}
	output_file_name += "log";
	outputFile = fopen(output_file_name.c_str(), "w");	// Open the file, clear the contents

	/* START OF PRODCON */
	/* SPAWN THE CONSUMER THREADS AND SAVE THEIR STATES */

	workTaken.resize(nthreads);				// Resize the workTaken vector now that we know the size
	buffer.resize(2 * nthreads);			// Resize the buffer, essentially initializing it

	program_start = chrono::duration_cast<chrono::milliseconds> (	// Store the program_start time
		chrono::system_clock::now().time_since_epoch()
	);

	/* SPAWN CONSUMERS */
	
	thread consumers[nthreads];

	for (int i = 0; i < nthreads; i++) {
		workTaken[i] = 0;
		consumers[i] = thread(thread_method, i + 1);
	}

	// Get input from stdin
	string line;
	char input[MAX_INPUT_LINE_LENGTH];
	int nq;

	while (fgets(input, MAX_INPUT_LINE_LENGTH, stdin) != NULL) {	// Read until EOF
				
		line = input;	// Convert to std::string
		
		int amount = stoi(line.substr(1));		// Length of 'work'
		string work_type = line.substr(0, 1);	// Type of 'work'

		if (work_type.compare("T") == 0) {		// If it's a transaction...

			buffer.push(amount, nq);			// Push to the queue
	
			printing_mutex.lock();				// Print that we're working
			n_characters_written = sprintf(output_str, "%8.3f ID= 0 Q= %d Work %8d\n", dTime(), nq, amount);
			fputs(output_str, outputFile);
			printing_mutex.unlock();
			nWork++;

		} else {								// It's a Sleep command (assumed)

			printing_mutex.lock();				// Print that we're sleeping
			n_characters_written = sprintf(output_str, "%8.3f ID= 0      Sleep %7d\n", dTime(), amount);
			fputs(output_str, outputFile);
			printing_mutex.unlock();

			Sleep(amount);						// Sleep

			nSleep++;
		}

	}

	/* STOP CONSUMERS */
	
	buffer.push(NO_MORE_WORK, nq);				// Signal the consumers there is no more work to be done
	for (int i = 0; i < nthreads; i++) {		// Wait for all consumers to complete
		consumers[i].join();
	}

	/* SUMMARY SECTION */
	
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

	fclose(outputFile);		// Close the output file

	return 0;
}
