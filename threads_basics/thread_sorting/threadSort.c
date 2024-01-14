#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

// max number of values for each thread to sort
#define MAX_VALS_PER_THREAD 1000
// max threads allowed
#define MAX_N_SIZE 100


/* other global variable instantiations can go here */

/* Uses a brute force method of sorting the input list. */
void BruteForceSort(int inputList[], int inputLength) {
  int i, j, temp;
  for (i = 0; i < inputLength; i++) {
    for (j = i+1; j < inputLength; j++) {
      if (inputList[j] < inputList[i]) {
        temp = inputList[j];
        inputList[j] = inputList[i];
        inputList[i] = temp;
      }
    }
  }
}

/* Uses the bubble sort method of sorting the input list. */
void BubbleSort(int inputList[], int inputLength) {
  char sorted = 0;
  int i, temp;
  while (!sorted) {
    sorted = 1;
    for (i = 1; i < inputLength; i++) {
      if (inputList[i] < inputList[i-1]) {
        sorted = 0;
        temp = inputList[i-1];
        inputList[i-1] = inputList[i];
        inputList[i] = temp;
      }
    }
  }
}

/* Merges two arrays.  Instead of having two arrays as input, it
 * merges positions in the overall array by re-ording data.  This 
 * saves space. */
void Merge(int *array, int left, int mid, int right) {
  int tempArray[MAX_VALS_PER_THREAD];
  int pos = 0, lpos = left, rpos = mid;
  while (lpos <= mid && rpos <= right) {
    if (array[lpos] < array[rpos]) {
      tempArray[pos++] = array[lpos++];
    } else {
      tempArray[pos++] = array[rpos++];
    }
  }
  while (lpos < mid)  tempArray[pos++] = array[lpos++];
  while (rpos <= right)tempArray[pos++] = array[rpos++];
  int iter;
  for (iter = 0; iter < pos; iter++) {
    array[iter+left] = tempArray[iter];
  }
  return;
}

/* Divides an array into halfs to merge together in order. */
void MergeSort(int array[], int inputLength) {
  int mid = inputLength/2;
  if (inputLength > 1) {
    MergeSort(array, mid);
    MergeSort(array + mid, inputLength - mid);
    // merge's last input is an inclusive index
    //printf("calling merge 0->%d, 1->%d\n mid %d\n",array[0], array[1], mid); 
    Merge(array, 0, mid, inputLength - 1);
  }
}

// you might want some globals, put them here

// here's a global I used you might find useful
char* descriptions[] = {"brute force","bubble","merge"};
typedef void (*sorter_t)(int[], int);
sorter_t sorters[3] = {&BruteForceSort, &BubbleSort, &MergeSort};
int vals_per_thread;
int *data_array;
pthread_mutex_t lock;

// I wrote a function called thread dispatch which parses the thread
// parameters and calls the correct sorting function
//
// you can do it a different way but I think this is easiest
void* thread_dispatch(void* data) {
  int threadNum = *(int*)data;
  int start = threadNum * vals_per_thread;
  char thread_task[55];
  sprintf(thread_task, "Sorting indices %d-%d with %s", start, start + vals_per_thread, descriptions[threadNum % 3]);
  printf("%s\n", thread_task);
  suseconds_t *time_taken = malloc(sizeof(suseconds_t));
  if(time_taken == NULL){
    printf("Malloc error at thread %d\n", threadNum);
    exit(1);
  }

  struct timeval startt, stopt;
  pthread_mutex_lock(&lock);
  gettimeofday(&startt, NULL);
  (sorters[threadNum % 3])(&data_array[start], vals_per_thread);
  gettimeofday(&stopt, NULL);
  pthread_mutex_unlock(&lock);
  *time_taken = (stopt.tv_usec < startt.tv_usec ? 1000000ul : 0ul) + stopt.tv_usec - startt.tv_usec;

  printf("%s done in %lu usecs\n", thread_task, *time_taken);
  pthread_exit((void*)time_taken);
}

int main(int argc, char** argv) {

  if(argc < 3) {
    printf("not enough arguments!\n");
    exit(1);
  }
  // I'm reading the value n (number of threads) for you off the
  // command line
  int n = atoi(argv[1]);
  if(n <= 0 || n > MAX_N_SIZE || n % 3 != 0) {
    printf("bad n value (number of threads) %d.  Must be a multiple of 3.\n", n);
    exit(1);
  }

  // I'm reading the number of values you want per thread
  // off the command line
  vals_per_thread = atoi(argv[2]);
  if(vals_per_thread <= 0 || vals_per_thread > MAX_VALS_PER_THREAD) {
    printf("bad vals_per_thread value %d\n", vals_per_thread);
    exit(1);
  }

  int total_nums = n * vals_per_thread;
  data_array = malloc(sizeof(int) * total_nums);
  if(data_array == NULL) {
    perror("malloc failure\n");
    exit(1);
  }

  // initialize the test data for sort
  for(int i = 0; i < total_nums; i++) {
    // big reverse sorted list
    data_array[i] = total_nums - i;
    // the test would be more traditional if we used random
    // values, but this makes it easier for you to visually
    // inspect and ensure you're sorting everything
  }

  // create your threads here
  pthread_t threads[n];
  int *arg;
  for(int i = 0; i < n; i++){
    arg = malloc(sizeof(int));
    if(arg == NULL){
      perror("Malloc failure\n");
      exit(1);
    }
    *arg = i;    
    pthread_create(&threads[i], NULL, thread_dispatch, arg);
  }

  unsigned long max_times[3] = {0, 0, 0}, min_times[3] = {0, 0, 0};
  int type;
  double avg_times[3] = {0, 0, 0};
  suseconds_t *result = malloc(sizeof(suseconds_t));

  for(int i = 0; i < n; i++){
    type = i % 3;
    pthread_join(threads[i], (void**)&result);
    if(i < 3 || min_times[type] > *result){
      min_times[type] = (int)*result;
    }
    if(max_times[type] < *result){
      max_times[type] = (int)*result;
    }
    avg_times[type] += (((double)*result)*3.0f/((double)n));
  }
  pthread_mutex_destroy(&lock);
  free(result);
  // wait for them to finish
  printf("brute force avg %f min %lu max %lu\n", avg_times[0], min_times[0], max_times[0]);
  printf("bubble avg %f min %lu max %lu\n", avg_times[1], min_times[1], max_times[1]); 
  printf("merge avg %f min %lu max %lu\n", avg_times[2], min_times[2], max_times[2]);
  
  // print out the algorithm summary statistics
  
  // print out the result array so you can see the sorting is working
  // you might want to comment this out if you're testing with large data sets
  // printf("Result array:\n");
  // for(int i = 0; i < n; i++) {
  // for(int j = 0; j < vals_per_thread; j++) {
  //   printf("%d ", data_array[i*vals_per_thread + j]);
  //  }
  //  printf("\n");
  //  }

  free(data_array); // we should free what we malloc
}
