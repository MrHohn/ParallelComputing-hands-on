#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <algorithm>
#include <math.h>
#include "immintrin.h"
// Using the same timing functions as the sample mandelbrot of ISPC
#include "timing.h"
// Include the header file that the ispc compiler generates
#include "sqrt_ispc.h"
using namespace ispc;
using namespace std;

extern void sqrt_serial(int N, float* nums, float* result);
extern void sqrt_avx(int N, float* nums, float* result);
extern void sqrt_test_iterations(int N, float* nums, float* result, int num_vector, int &total_serial, int &total_simd);

// function to judge the correctness of the result
bool check(float* exactRes, float* newRes, int len)
{
	for (int i = 0; i < len; ++i)
	{
		if (fabs(exactRes[i] - newRes[i]) > 1e-4)
			return false;
	}

	return true;
}

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
	// define the total iteration of testing
	static int test_iteration = 3;
	// define the number of vectors when do data parallelism
	static int num_vector = 8;

	// generate all the random numbers first
	// loop for 20 millions time -> 20,000,000
	int totalNum = 20 * 1000 * 1000;
	// int totalNum = 8;

	// allocate memory space for the inputs and results in heap
	float* nums = (float*) _mm_malloc(totalNum * sizeof(float), 32);
	float* result = (float*) _mm_malloc(totalNum * sizeof(float), 32);
	// keep a copy of the result to check the correctness
	float* result_exact = (float*) malloc(totalNum * sizeof(float));


	// initiallize the random seed by current time to avoid duplicate
	srand(time(NULL));
	// generate random numbers and calculate the real sqrt
	for (int i = 0; i < totalNum; ++i)
	{
	    nums[i] = randNum(0, 3);
	    // printf("input number is: %f\n", num[i]);
	    result_exact[i] = sqrt(nums[i]);
	    result[i] = 0;
	}

	printf("\n------- CALCULATE TOTAL ITERATIONS OF SERIAL AND SIMD -------\n\n");
	int total_serial, total_simd;
	// call the test function to calculate the total #
	sqrt_test_iterations(totalNum, nums, result, num_vector, total_serial, total_simd);
	printf("total iterations of serial: \t\t%d\n", total_serial);
	printf("total iterations of SIMD: \t\t%d\n", total_simd);
	printf("\nThe theoretical speedup for SIMD is: \t%f\n\n", (float)total_serial / (float) total_simd);

	printf("----------------------- SQRT TEST BEGIN ---------------------\n\n");
	// clear the result buffer
	for (int i = 0; i < totalNum; ++i)
	{
		result[i] = 0;
	}

	printf("Run the serial version first...\n");
	double minSerial = 1e30;
	for (int i = 0; i < test_iteration; ++i)
	{
		// start to record time consumption
		reset_and_start_timer();
		// call the sqrt_serial function to calculate all the inputs
		sqrt_serial(totalNum, nums, result);
		// stop timer and print out total cycles
		double one_round = get_elapsed_mcycles();
		printf("time of serial run %d:\t\t\t[%.3f] million cycles\n", i + 1, one_round);
		minSerial = min(minSerial, one_round);
	}
	printf("[best of sqrt_serial]:\t\t\t[%.3f] million cycles\n", minSerial);

	// now check the result
	printf("Now check the correctness...");
	if (check(result_exact, result, totalNum))
		printf("\t\tOutput correct!\n\n");
	else
		printf("\t\tOutput incorrect!\n\n");	

	printf("------------------------- NEXT: ISPC -------------------------\n\n");
	// clear the result buffer
	for (int i = 0; i < totalNum; ++i)
	{
		result[i] = 0;
	}

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
		printf("time of ISPC run %d:\t\t\t[%.3f] million cycles\n", i + 1, one_round);
		minISPC = min(minISPC, one_round);
	}
	printf("[best of sqrt_ispc]:\t\t\t[%.3f] million cycles\n", minISPC);
	
	// calculate the speedup
	printf("\t\t\t\t\t(%.2fx speedup from ISPC avx1-i32x8)\n", minSerial / minISPC);

	// now check the result
	printf("Now check the correctness...");
	if (check(result_exact, result, totalNum))
		printf("\t\tOutput correct!\n\n");
	else
		printf("\t\tOutput incorrect!\n\n");

	printf("-------------------- NEXT: ISPC WITH TASKS -------------------\n\n");
	// clear the result buffer
	for (int i = 0; i < totalNum; ++i)
	{
		result[i] = 0;
	}

	printf("Now run the ISPC with tasks...\n");
	double minISPCtask = 1e30;
	for (int i = 0; i < test_iteration; ++i)
	{
		// start to record time consumption
		reset_and_start_timer();
		// call the sqrt_ispc function to calculate all the inputs
		sqrt_ispc_task(totalNum, nums, result);
		// stop timer and print out total cycles
		double one_round = get_elapsed_mcycles();
		printf("time of ISPC run %d:\t\t\t[%.3f] million cycles\n", i + 1, one_round);
		minISPCtask = min(minISPCtask, one_round);
	}
	printf("[best of sqrt_ispc_task]:\t\t[%.3f] million cycles\n", minISPCtask);
	
	// calculate the speedup
	printf("\t\t\t\t\t(%.2fx speedup with tasks on 4 cores)\n", minSerial / minISPCtask);

	// now check the result
	printf("Now check the correctness...");
	if (check(result_exact, result, totalNum))
		printf("\t\tOutput correct!\n\n");
	else
		printf("\t\tOutput incorrect!\n\n");

	printf("--------------------- NEXT: AVX INTRINSICS -------------------\n\n");
	// clear the result buffer
	for (int i = 0; i < totalNum; ++i)
	{
		result[i] = 0;
	}

	printf("Now run the AVX intrinsics...\n");
	double minAVX = 1e30;
	for (int i = 0; i < test_iteration; ++i)
	{
		// start to record time consumption
		reset_and_start_timer();
		// call the sqrt_ispc function to calculate all the inputs
		sqrt_avx(totalNum, nums, result);
		// stop timer and print out total cycles
		double one_round = get_elapsed_mcycles();
		printf("time of AVX run %d:\t\t\t[%.3f] million cycles\n", i + 1, one_round);
		minAVX = min(minAVX, one_round);
	}
	printf("[best of sqrt_avx]:\t\t\t[%.3f] million cycles\n", minAVX);
	
	// calculate the speedup
	printf("\t\t\t\t\t(%.2fx speedup from 8-wide AVX)\n", minSerial / minAVX);

	// now check the result
	printf("Now check the correctness...");
	if (check(result_exact, result, totalNum))
		printf("\t\tOutput correct!\n\n");
	else
		printf("\t\tOutput incorrect!\n\n");

	printf("-------------------------- TEST END --------------------------\n\n");

	// now free the memory
	delete[] nums;
	delete[] result;
	delete[] result_exact;

	return 0;
}
