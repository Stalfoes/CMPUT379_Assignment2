#include <cstdio>	// perror, fgets, printf
#include <cstdlib>	// atoi
#include <cctype>	// isdigit
#include <string>

/* Declare the external C file's functions */
extern "C" {
	void Trans(int n);
	void Sleep(int n);
};

using namespace std;

int main(int argc, char *argv[]) {

	/* GET PARAMETERS */

	if (argc < 2) {
		perror("prodcon requires at least 1 argument. prodcon nthreads <id>\n");
		return 1;
	}

	int nthreads = atoi(argv[1]);
	string id = "0";

	if (argc >= 3) {
		if ( isdigit( argv[2][0] ) ) {
			id = argv[2];
		}
	}

	printf("You entered details NTHREADS = %d, ID = %s", nthreads, id);

	/* START THE PROCESS */
	/* READ FROM STDIN AND PERFORM THE TASKS */

	return 0;
}
