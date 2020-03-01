#include <cstdio>
#include <ctime>
#include <chrono>
#include <thread>

using namespace std;

int main() {

	clock_t startClock = clock();
	
	chrono::milliseconds startTime = chrono::duration_cast<chrono::milliseconds> ( 
		chrono::system_clock::now().time_since_epoch()
	);

	this_thread::sleep_for(chrono::seconds(3));

	float dc = ((float)(clock() - startClock)) / CLOCKS_PER_SEC;

	chrono::milliseconds dt_ms = chrono::duration_cast<chrono::milliseconds> ( 
		chrono::system_clock::now().time_since_epoch()
	) - startTime;

	float dt = ((float) dt_ms.count()) / 1000;

	printf("CPU:  %.5f\nREAL: %.5f\n", dc, dt);

	return 0;
}
