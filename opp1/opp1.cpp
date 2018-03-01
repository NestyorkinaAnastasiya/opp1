#include <stdio.h>
#include <stdlib.h>
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <ctime>
#include <chrono>

// n - count of strings
// m - count of columns
const int	n = 5000, 
			m = 5000,
			r = 5000;
const int dim = n * m;
const int dim2 = m * r;
const int dim3 = n * r;
int m1[dim];
int m2[dim];
int res[dim3];


// count of strings for thread
int ids[16] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
pthread_t thrs[16];
int countOfWorkers = 1;
int countOfThreads = 1;
int fragmentSize = n / countOfWorkers;

int globalRes = 0, finalRes = 0;

pthread_mutex_t mutex;

void init()
{
	for (int i = 0; i < n*m; i++)
		m1[i] = 1;
	for (int i = 0; i < m*r; i++)
		m2[i] = 1;
}

void* worker(void* me)
{
	int id = *((int*)me);
	int localRes = 0;
	// index of the first element of current string
	int offset_i = id * m * fragmentSize;
	// -//- column
	int offset_j = 0;
	int offset_r = id * r * fragmentSize;

	// all elements of strings
	for (int j = 0; j < fragmentSize*r; j++)
	{
		localRes = 0;
		// multiply current string and current column
		for (int i = offset_i, k = offset_j; i < offset_i + m; i++)
		{
			localRes += m1[i] * m2[k];
			k += r;
		}

		pthread_mutex_lock(&mutex);
		res[offset_r + offset_j] = localRes;
		pthread_mutex_unlock(&mutex);

		offset_j++;
		if (offset_j == r)
		{
			offset_i += m;
			offset_r += r;
			offset_j = 0;
		}
	}
	return 0;
}


int main(int argc, char **argv)
{
	unsigned int beginTime = clock();	
	auto t_start = std::chrono::high_resolution_clock::now();
	pthread_attr_t attrs;
	init();
	if (0 != pthread_attr_init(&attrs))
	{
		perror("Cannot initialize attributes");
		abort();
	};
	// Инициализация мьютекса
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutex_init(&mutex, &attr);

	if (0 != pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE))
	{
		perror("Error in setting attributes");
		abort();
	}

	for (int i = 0; i < countOfWorkers; i++)
		if (0 != pthread_create(&thrs[i], &attrs, worker, &ids[i]))
		{
			perror("Cannot create a thread");
			abort();
		}
	pthread_attr_destroy(&attrs);

	for (int i = 0; i < countOfThreads; i++)
		if (0 != pthread_join(thrs[i], NULL))
		{
			perror("Cannot join a thread");
			abort();
		}
	auto t_end = std::chrono::high_resolution_clock::now();
	unsigned int endTime = clock();
/*	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < r; j++)
			printf("%d ", res[i*r + j]);
		printf("\n");
	}*/
	printf("Time: %d\t%f", (endTime - beginTime)*1000/CLOCKS_PER_SEC, std::chrono::duration<double, std::milli>(t_end - t_start).count());
	getchar();
	return 0;
}
