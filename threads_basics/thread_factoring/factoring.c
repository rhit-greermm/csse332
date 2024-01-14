/* Copyright 2016 Rose-Hulman Institute of Technology

   Here is some code that factors in a super dumb way.  We won't be
   attempting to improve the algorithm in this case (though that would be
   the correct thing to do).

   Modify the code so that it starts the specified number of threads and
   splits the computation among them.  You can be sure the max allowed 
   number of threads is 50.  Be sure your threads actually run in parallel.

   Your threads should each just print the factors they find, they don't
   need to communicate the factors to the original thread.

   ALSO - be sure to compile this code with -pthread or just used the 
   Makefile provided.

*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#include <stdlib.h>

int numThreads;
unsigned long long int target;

int main(void);
void *findFactors(void*);

int main(void) {
  /* you can ignore the linter warning about this */
  printf("Give a number to factor.\n");
  scanf("%llu", &target);

  printf("How man threads should I create?\n");
  scanf("%d", &numThreads);
  if (numThreads > 50 || numThreads < 1) {
    printf("Bad number of threads!\n");
    return 0;
  }
  pthread_t threads[numThreads];
  int *arg;
  for(int j = 0; j < numThreads; j++){
    arg = malloc(sizeof(int));
    if(arg == NULL){
      perror("Malloc error");
      exit(1);
    }
    *arg = j;
    pthread_create(&threads[j], NULL, findFactors, arg);    
  }
  for(int j = 0; j < numThreads; j++){
    pthread_join(threads[j], NULL);
  }
}
  
void *findFactors(void *arg) {
  int threadNum = *(int*)arg + 1;
  for (unsigned long long int i = threadNum + 1; i <= target/2; i += numThreads) {
    /* You'll want to keep this testing line in.  Otherwise it goes so
       fast it can be hard to detect your code is running in
       parallel. Also test with a large number (i.e. > 3000) */
    printf("thread %d testing %llu\n", threadNum, i);
    if (target % i == 0) {
      printf("%llu is a factor\n", i);
    }
  }
  pthread_exit(NULL);
}


