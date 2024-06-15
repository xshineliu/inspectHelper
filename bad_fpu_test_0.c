/*
* 
* x86_64-x86_64-clang-1101/bin/clang -g  -o bad_fpu_test_0 bad_fpu_test_0.c -lpthread
* 
* x86_64-x86_64-clang-1101/bin/clang -g -O2 -o bad_fpu_test_0_O2 bad_fpu_test_0.c -lpthread
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
int aff = 0;

void *work(void *parm) {
  int val = *(int *)parm;
  int s = 0;
  unsigned long long loop = 0ULL;
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(aff, &cpuset);

  pthread_t thread;
  thread = pthread_self();

  s = pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);
  if (s != 0) {
      error(EXIT_FAILURE, s, "pthread_setaffinity_np %d", aff);
  }
  while (1) {
    double x = foo(m);
    if (x < 0.0) {
      printf("%4d HIT HIT HIT !!! %llx %f loop=%llx\n", aff, *(unsigned long long*)&x, x, loop);
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
    aff = atoi(argv[1]);
  }

  //total_online = sysconf(_SC_NPROCESSORS_ONLN);
  //printf("Online %d of max %d, aff config %d\n", total_online, N, aff);
  total_online = 1;

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
