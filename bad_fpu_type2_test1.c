#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <immintrin.h>
#include <unistd.h>
#include <err.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

double __attribute__((noinline)) foo(size_t m) { return m; }

double __attribute__((noinline)) foo2(size_t m) {

  __m128i magic1;
  __m128i magic2;
  double val_ret = 0.0f;

  unsigned int mh = 0x45300000;
  unsigned int ml = 0x43300000;


  magic1 = _mm_set_epi32(0x0, 0x0, mh, ml);
  magic2 = _mm_set_epi32(mh, 0x0, ml, 0);

   __asm__ __volatile__ ("movaps %2,%%xmm0\n\t"
"__BRKP:\n\t"
        "movq %1,%%xmm1\n\t"
        "punpckldq %%xmm0,%%xmm1\n\t"
	"movapd %3,  %%xmm0\n\t"
	"subpd  %%xmm0,%%xmm1\n\t"
	"pshufd $0x4e,%%xmm1,%%xmm0\n\t"
	"addpd  %%xmm1,%%xmm0\n\t"
        "movsd  %%xmm0, %0\n\t"
        : "=m"(val_ret)
        : "m"(m), "m"(magic1), "m"(magic2)
        : "xmm0", "xmm1");

    return val_ret;
}


double __attribute__((noinline)) foo3(size_t m) {

  __m128i magic1;
  __m128i magic2;
  double val_ret = 0.0f;

  unsigned int mh = 0x45300000;
  unsigned int ml = 0x43300000;


  magic1 = _mm_set_epi32(0x0, 0x0, mh, ml);
  magic2 = _mm_set_epi32(mh, 0x0, ml, 0);

   __asm__ __volatile__ ("movaps %2,%%xmm0\n\t"
"__BRKP3:\n\t"
        "movq %1,%%xmm1\n\t"
        "punpckldq %%xmm0,%%xmm1\n\t"
        "pshufd $0xe4, %%xmm1,%%xmm2\n\t"
	"movapd %3,  %%xmm0\n\t"
	"subpd  %%xmm0, %%xmm1\n\t"
	"movapd %%xmm1,  %%xmm3\n\t"
	"pshufd $0x4e,%%xmm1,%%xmm0\n\t"
	"movapd %%xmm0,  %%xmm4\n\t"
	"addpd  %%xmm1,%%xmm0\n\t"
        "movsd  %%xmm0, %0\n\t"
        : "=m"(val_ret)
        : "m"(m), "m"(magic1), "m"(magic2)
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4");

    return val_ret;
}


double __attribute__((noinline)) foo4(size_t m) {

  __m128i magic1;
  __m128i magic2;
  double val_ret = 0.0f;

  unsigned int mh = 0x45300000;
  unsigned int ml = 0x43300000;


  magic1 = _mm_set_epi32(0x0, 0x0, mh, ml);
  magic2 = _mm_set_epi32(mh, 0x0, ml, 0);

   __asm__ __volatile__ ("movaps %2,%%xmm0\n\t"
"__BRKP4:\n\t"
        "movq %1,%%xmm1\n\t"
        "punpckldq %%xmm0,%%xmm1\n\t"
        "pshufd $0xe4, %%xmm1,%%xmm2\n\t"
	"movapd %3,  %%xmm0\n\t"
	"subpd  %%xmm0, %%xmm1\n\t"
	"pshufd $0x4e,%%xmm1,%%xmm0\n\t"
	"addpd  %%xmm1,%%xmm0\n\t"
        "movsd  %%xmm0, %0\n\t"
        : "=m"(val_ret)
        : "m"(m), "m"(magic1), "m"(magic2)
        : "xmm0", "xmm1", "xmm2");

    return val_ret;
}

double __attribute__((noinline)) foo5(size_t m) {

  __m128i magic1;
  __m128i magic2;
  double val_ret = 0.0f;

  unsigned int mh = 0x45300000;
  unsigned int ml = 0x43300000;


  magic1 = _mm_set_epi32(0x0, 0x0, mh, ml);
  magic2 = _mm_set_epi32(mh, 0x0, ml, 0);

   __asm__ __volatile__ ("movups %2,%%xmm0\n\t"
"__BRKP5:\n\t"
        "movq %1,%%xmm1\n\t"
        "punpckldq %%xmm0,%%xmm1\n\t"
	"movupd %3,  %%xmm0\n\t"
	/* "pshufd $0xe4, %%xmm1,%%xmm2\n\t" */
	"subpd  %%xmm0, %%xmm1\n\t"
	"movapd %%xmm1,  %%xmm0\n\t"
	"unpckhpd  %%xmm1, %%xmm1\n\t"
	"addpd  %%xmm1,%%xmm0\n\t"
        "movsd  %%xmm0, %0\n\t"
        : "=m"(val_ret)
        : "m"(m), "m"(magic1), "m"(magic2)
        : "xmm0", "xmm1", "xmm2");


    return val_ret;
}


double __attribute__((noinline)) foo6(size_t m) {

  __m128i magic1;
  __m128i magic2;
  double val_ret = 0.0f;

  unsigned int mh = 0x45300000;
  unsigned int ml = 0x43300000;


  magic1 = _mm_set_epi32(0x0, 0x0, mh, ml);
  magic2 = _mm_set_epi32(mh, 0x0, ml, 0);

   __asm__ __volatile__ ("movups %2,%%xmm0\n\t"
"__BRKP6:\n\t"
        "movq %1,%%xmm1\n\t"
        "punpckldq %%xmm0,%%xmm1\n\t"
	"movupd %3,  %%xmm0\n\t"
	"subpd  %%xmm0, %%xmm1\n\t"
	"movapd %%xmm1,  %%xmm0\n\t"
	"unpckhpd  %%xmm0, %%xmm0\n\t"
        "movsd  %%xmm0, %0\n\t"
        : "=m"(val_ret)
        : "m"(m), "m"(magic1), "m"(magic2)
        : "xmm0", "xmm1", "xmm2");


    return val_ret;
}


double __attribute__((noinline)) foo7(size_t m) {

  __m128i magic1;
  __m128i magic2;
  __m128i magic3;
  double val_ret = 0.0f;

  unsigned int mh = 0x45300000;
  unsigned int ml = 0x43300000;


  magic1 = _mm_set_epi32(mh, 0x0, ml, (unsigned int)m);
  magic2 = _mm_set_epi32(mh, 0x0, ml, 0);

   __asm__ __volatile__ ("nop\n\t"
"__BRKP7:\n\t"
        "movupd %1, %%xmm1\n\t"
	"movupd %2,  %%xmm0\n\t"
	"subpd  %%xmm0, %%xmm1\n\t"
	"movapd %%xmm1,  %%xmm0\n\t"
	"unpckhpd  %%xmm0, %%xmm0\n\t"
        "movsd  %%xmm0, %0\n\t"
        : "=m"(val_ret)
        : "m"(magic1), "m"(magic2)
        : "xmm0", "xmm1", "xmm2");


    return val_ret;
}


double __attribute__((noinline)) fooX(size_t m) {

  __m128i magic1;
  __m128i magic2;
  double val_ret = 0.0f;

  unsigned int mh = 0x45300000;
  unsigned int ml = 0x43300000;


  magic1 = _mm_set_epi32(mh, 0x0, ml, 0x29);
  magic2 = _mm_set_epi32(mh, 0x0, ml, 0);

   __asm__ __volatile__ ("movupd %2, %%xmm0\n\t"
"__BRKPX:\n\t"
        "movupd %1, %%xmm1\n\t"
	"subpd  %%xmm0, %%xmm1\n\t"
	"pshufd $0xe4,%%xmm1,%%xmm0\n\t"
        "movsd  %%xmm0, %0\n\t"
        : "=m"(val_ret)
        : "m"(magic1), "m"(magic2)
        : "xmm0", "xmm1");

    return val_ret;
}





size_t m = 41;

#define N 1024
int total_online = 0;
int infinite = -1;

void *work(void *parm) {
  int val = *(int *)parm;
  int s = 0;
  unsigned long long loop = 0ULL;
  unsigned long long loop_hit = 0ULL;
  cpu_set_t cpuset;
  pthread_t thread;

  CPU_ZERO(&cpuset);
  CPU_SET(val, &cpuset);
  thread = pthread_self();


  s = pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);
  if (s != 0) {
      error(EXIT_FAILURE, s, "pthread_setaffinity_np %d", val);
  }
  fprintf(stderr, "pthread_setaffinity_np %d succussfull\n", val);
  while (1) {
    //double x = foo(m);
    double x = foo7(m);
__XXX:
    if (x < 0.0) {
      loop_hit++;
      printf("%4d HIT HIT HIT !!! %llx %f loop=%llx hit=%llx\n", val, *(unsigned long long*)&x, x, loop, loop_hit);
      //break;
    }
    if ( infinite < 0) {
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

  //total_online = sysconf(_SC_NPROCESSORS_ONLN);
  total_online = 1;
  printf("Online %d of max %d, infinite config %d\n", total_online, N, infinite);

  if ( total_online > N ) {
      error(EXIT_FAILURE, N, "please recompile the app");
  }

  for (int i = 0; i < total_online; i++) {
    index_id[i] = i;
    pthread_create(&a[i], NULL, work, &infinite);
    //usleep(1000 * 1);
  }

  for (int i = 0; i < total_online; i++) {
    pthread_join(a[i], NULL);
  }

  exit(EXIT_SUCCESS);
}
