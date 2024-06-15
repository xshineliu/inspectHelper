/* x86_64-x86_64-clang-1101/bin/clang -g -o bad_fpu_test_1 bad_fpu_test_1.c  -lpthread
* 
* ./bad_fpu_test_1
* Online 96 of max 1024, infinite config 0
*   42 HIT HIT HIT !!! c330000000000000 -4503599627370496.000000 loop=0
*   90 HIT HIT HIT !!! c330000000000000 -4503599627370496.000000 loop=0
* 
*/


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <err.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

double __attribute__((noinline)) foo(size_t m) { return m; }

size_t m = 41;

#define N 1024
int total_online = 0;
int infinite = 0;

void *work(void *parm) {
  int val = *(int *)parm;
  int s = 0;
  unsigned long long loop = 0ULL;
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(val, &cpuset);

  pthread_t thread;
  thread = pthread_self();

  if(val >= total_online) {
     printf("Error thread_index = %d, total_online = %d\n", val, total_online);
     return NULL;
  }

  s = pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);
  if (s != 0) {
      error(EXIT_FAILURE, s, "pthread_setaffinity_np %d", val);
  }
  while (1) {
    double x = foo(m);
    if (x < 0.0) {
      printf("%4d HIT HIT HIT !!! %llx %f loop=%llx\n", val, *(unsigned long long*)&x, x, loop);
      break;
    }
    if ( infinite == 0) {
      break;
    }
    loop++;
  }
  return NULL;
}

int main(int argc, char* argv[]) {

  pthread_t a[N];
  int index_id[N];

  if(argc > 1) {
    infinite = atoi(argv[1]);
  }

  total_online = sysconf(_SC_NPROCESSORS_ONLN);
  printf("Online %d of max %d, infinite config %d\n", total_online, N, infinite);

  if ( total_online > N ) {
      error(EXIT_FAILURE, N, "please recompile the app");
  }

  for (int i = 0; i < total_online; i++) {
    index_id[i] = i;
    pthread_create(&a[i], NULL, work, &(index_id[i]));
    //usleep(1000 * 1);
  }

  for (int i = 0; i < total_online; i++) {
    pthread_join(a[i], NULL);
  }

  exit(EXIT_SUCCESS);
}
