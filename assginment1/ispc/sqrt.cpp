#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <algorithm>
#include "timing.h"
// Include the header file that the ispc compiler generates
#include "sqrt_ispc.h"
using namespace ispc;
using namespace std;

extern void sqrt_serial(int N, float* nums, float* result);

// function to generate random float number between rMin and rMax
float randNum(float rMin, float rMax)
{
	// use the built-in rand() to generate random int number
	// convert it into fraction of 1
	float ranFraction = (float)rand() / RAND_MAX;
	// now convert the result to between range[rMin, rMax]
	return rMin + ranFraction * (rMax - rMin);
}

int main()
{
	static int test_iteration = 3;

	// generate all the random numbers first
	// loop for 20 millions time -> 20,000,000
	int totalNum = 20000000;
	// allocate memory space for the inputs and results in heap
	float* nums = (float*) malloc(totalNum * sizeof(float));
	float* result = (float*) malloc(totalNum * sizeof(float));
	// initiallize the random seed by current time to avoid duplicate
	srand(time(NULL));
	// generate random numbers
	for (int i = 0; i < totalNum; ++i)
	{
	    nums[i] = randNum(0, 3);
	    // printf("input number is: %f\n", num[i]);
	}
	
	printf("\nRun the serial version first...\n");
	double minSerial = 1e30;
	for (int i = 0; i < test_iteration; ++i)
	{
		// start to record time consumption
		reset_and_start_timer();
		// call the sqrt_serial function to calculate all the inputs
		sqrt_serial(totalNum, nums, result);
		// stop timer and print out total cycles
		double one_round = get_elapsed_mcycles();
		printf("time of serial run:\t\t\t[%.3f] million cycles\n", one_round);
		minSerial = min(minSerial, one_round);
	}
	printf("[best of sqrt_serial]:\t\t\t[%.3f] million cycles\n\n", minSerial);

	// print the inputs and outputs
	// for (int i = 0; i < totalNum; ++i) {
	// 	printf("input: %f, output: %f\n", nums[i], result[i]);
	// }

	// Clear out the result buffer
    for (int i = 0; i < totalNum; ++i)
        result[i] = 0;

	printf("Now run the ISPC version...\n");
	double minISPC = 1e30;
	for (int i = 0; i < test_iteration; ++i)
	{
		// start to record time consumption
		reset_and_start_timer();
		// call the sqrt_ispc function to calculate all the inputs
		sqrt_ispc(totalNum, nums, result);
		// stop timer and print out total cycles
		double one_round = get_elapsed_mcycles();
		printf("time of ISPC run:\t\t\t[%.3f] million cycles\n", one_round);
		minISPC = min(minISPC, one_round);
	}
	printf("[best of sqrt_ISPC]:\t\t\t[%.3f] million cycles\n\n", minISPC);

	// print the inputs and outputs
	// for (int i = 0; i < totalNum; ++i) {
	// 	printf("input: %f, output: %f\n", nums[i], result[i]);
	// }

	printf("\t\t\t\t\t(%.2fx speedup from ISPC)\n", minSerial/minISPC);

	// now free the memory
	free(nums);
	free(result);

	return 0;
}
