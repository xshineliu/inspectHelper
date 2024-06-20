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

#define N 1024
#define LOOP 1000

size_t m = 41;

int total_online = 0;
_Atomic int hit_fault = 0;
int bind_core = 0;

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
"__BRKP2:\n\t"
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

double __attribute__((noinline)) foo8(size_t m) {

  __m128i magic1;
  __m128i magic2;
  __m128d magic3;

  double val_ret = 0.0f;

  unsigned int mh = 0x45300000;
  unsigned int ml = 0x43300000;


  magic1 = _mm_set_epi32(mh, 0x0, ml, (unsigned int)m);
  magic2 = _mm_set_epi32(mh, 0x0, ml, 0);

   __asm__ __volatile__ ("nop\n\t"
"__BRKP8:\n\t"
        "movupd %1, %%xmm1\n\t"
	"movupd %2,  %%xmm0\n\t"
	"subpd  %%xmm0, %%xmm1\n\t"
	"movupd %%xmm1, %0\n\t"
        : "=m"(magic3)
        : "m"(magic1), "m"(magic2)
        : "xmm0", "xmm1");

    val_ret = magic3[1];
    return val_ret;
}


__attribute__((noinline)) double foo9(size_t m) {

  __m128i res1;
  __m128i magic1;
  __m128i val1;

  unsigned int expector = 0xFFFFFFFF;
  unsigned int obtained = 0xFFFFFFFF;
  size_t hit1 = 0;
  size_t hit2 = 0;

  //for (; expector != 0; expector--) {
  expector = 0x00551234;
  {
    magic1 = _mm_set_epi32(0x0, 0x0, 0x45300000, expector);
    val1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x29);

    __asm__ __volatile__ ("movupd %1, %%xmm1\n\t"
        "movupd %2, %%xmm0\n\t"
        "punpckldq %%xmm0,%%xmm1\n\t"
  "__BRKH9:\n\t"
        "movdqu %%xmm1, %0\n\t"
        : "=m"(res1)
        : "m"(val1), "m"(magic1)
        : "xmm0", "xmm1");

    obtained = (unsigned int)(res1[0] >> 32);

    if(obtained != expector) {
      hit1++;

      if( (obtained >> 24) == ((obtained & 0x00FF0000) >> 16) ) {
        hit2++;
      }
    }

    if ( hit1 > 0 ) {
      return -1.0f;
    } else {
      return 1.0f;
    }
  }

}


void *work(void *parm) {
  int val = *(int *)parm;
  int s = 0;
  unsigned long long loop = 0ULL;
  unsigned long long loop_hit = 0ULL;
  double x1 = 0.0f;
  double x2 = 0.0f;
  double x3 = 0.0f;
  cpu_set_t cpuset;
  pthread_t thread;

  CPU_ZERO(&cpuset);
  CPU_SET(val, &cpuset);
  thread = pthread_self();


  s = pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);
  if (s != 0) {
      error(EXIT_FAILURE, s, "pthread_setaffinity_np %d", val);
  }
  //fprintf(stderr, "pthread_setaffinity_np %d succussfull\n", val);

  for (; loop < LOOP; loop++) {
    x1 = foo2(m);
    x2 = foo8(m);
    x3 = foo9(m);
    if (x1 < 0.0 || x2 < 0.0 || x3 < 0.0) {
      loop_hit++;
    }
  }

  if( loop_hit > 0) {
    hit_fault = 1;
    printf("HIT HIT HIT for logical processor %4d !!! loop=%20lld hit=%20lld\n", val, loop, loop_hit);
  }

  return NULL;
}



int main(int argc, char* argv[]) {

  pthread_t a[N];
  int index_id[N];

  if( argc > 1 ) {
    bind_core = atoi(argv[1]);
    total_online = 1;
  } else {
    total_online = sysconf(_SC_NPROCESSORS_ONLN);
    total_online = total_online / 2;
  }

  printf("Online %d x2HT of max %d, bind_core config %d\n", total_online, N, bind_core);

  if ( total_online > N ) {
      error(EXIT_FAILURE, N, "please recompile the app");
  }

  if (argc > 1) {
      pthread_create(&a[0], NULL, work, &bind_core);
  } else {
    for (int i = 0; i < total_online; i++) {
      index_id[i] = i;
      pthread_create(&a[i], NULL, work, &(index_id[i]));
      //usleep(1000 * 1);
    }
  }

  for (int i = 0; i < total_online; i++) {
    pthread_join(a[i], NULL);
  }

  //exit(EXIT_SUCCESS);
  return hit_fault;
}

root@n192-171-034:~/shine# clang -g -O2 -o ./x86_fpu_sde_checker ./x86_fpu_sde_checker.c -mavx -lpthread
root@n192-171-034:~/shine# cat ./x86_fpu_sde_checker.c
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

#define N 1024
#define LOOP 1000

size_t m = 41;

int total_online = 0;
_Atomic int hit_fault = 0;
int bind_core = 0;

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
"__BRKP2:\n\t"
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

double __attribute__((noinline)) foo8(size_t m) {

  __m128i magic1;
  __m128i magic2;
  __m128d magic3;

  double val_ret = 0.0f;

  unsigned int mh = 0x45300000;
  unsigned int ml = 0x43300000;


  magic1 = _mm_set_epi32(mh, 0x0, ml, (unsigned int)m);
  magic2 = _mm_set_epi32(mh, 0x0, ml, 0);

   __asm__ __volatile__ ("nop\n\t"
"__BRKP8:\n\t"
        "movupd %1, %%xmm1\n\t"
	"movupd %2,  %%xmm0\n\t"
	"subpd  %%xmm0, %%xmm1\n\t"
	"movupd %%xmm1, %0\n\t"
        : "=m"(magic3)
        : "m"(magic1), "m"(magic2)
        : "xmm0", "xmm1");

    val_ret = magic3[1];
    return val_ret;
}


__attribute__((noinline)) double foo9(size_t m) {

  __m128i res1;
  __m128i magic1;
  __m128i val1;

  unsigned int expector = 0xFFFFFFFF;
  unsigned int obtained = 0xFFFFFFFF;
  size_t hit1 = 0;
  size_t hit2 = 0;

  //for (; expector != 0; expector--) {
  expector = 0x00551234;
  {
    magic1 = _mm_set_epi32(0x0, 0x0, 0x45300000, expector);
    val1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x29);

    __asm__ __volatile__ ("movupd %1, %%xmm1\n\t"
        "movupd %2, %%xmm0\n\t"
        "punpckldq %%xmm0,%%xmm1\n\t"
  "__BRKH9:\n\t"
        "movdqu %%xmm1, %0\n\t"
        : "=m"(res1)
        : "m"(val1), "m"(magic1)
        : "xmm0", "xmm1");

    obtained = (unsigned int)(res1[0] >> 32);

    if(obtained != expector) {
      hit1++;

      if( (obtained >> 24) == ((obtained & 0x00FF0000) >> 16) ) {
        hit2++;
      }
    }

    if ( hit1 > 0 ) {
      return -1.0f;
    } else {
      return 1.0f;
    }
  }

}


void *work(void *parm) {
  int val = *(int *)parm;
  int s = 0;
  unsigned long long loop = 0ULL;
  unsigned long long loop_hit = 0ULL;
  double x1 = 0.0f;
  double x2 = 0.0f;
  double x3 = 0.0f;
  cpu_set_t cpuset;
  pthread_t thread;

  CPU_ZERO(&cpuset);
  CPU_SET(val, &cpuset);
  thread = pthread_self();


  s = pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);
  if (s != 0) {
      error(EXIT_FAILURE, s, "pthread_setaffinity_np %d", val);
  }
  //fprintf(stderr, "pthread_setaffinity_np %d succussfull\n", val);

  for (; loop < LOOP; loop++) {
    x1 = foo2(m);
    x2 = foo8(m);
    x3 = foo9(m);
    if (x1 < 0.0 || x2 < 0.0 || x3 < 0.0) {
      loop_hit++;
    }
  }

  if( loop_hit > 0) {
    hit_fault = 1;
    printf("HIT HIT HIT for logical processor %4d !!! loop=%20lld hit=%20lld\n", val, loop, loop_hit);
  }

  return NULL;
}



int main(int argc, char* argv[]) {

  pthread_t a[N];
  int index_id[N];

  if( argc > 1 ) {
    bind_core = atoi(argv[1]);
    total_online = 1;
  } else {
    total_online = sysconf(_SC_NPROCESSORS_ONLN);
    total_online = total_online / 2;
  }

  printf("Online %d x2HT of max %d, bind_core config %d\n", total_online, N, bind_core);

  if ( total_online > N ) {
      error(EXIT_FAILURE, N, "please recompile the app");
  }

  if (argc > 1) {
      pthread_create(&a[0], NULL, work, &bind_core);
  } else {
    for (int i = 0; i < total_online; i++) {
      index_id[i] = i;
      pthread_create(&a[i], NULL, work, &(index_id[i]));
      //usleep(1000 * 1);
    }
  }

  for (int i = 0; i < total_online; i++) {
    pthread_join(a[i], NULL);
  }

  //exit(EXIT_SUCCESS);
  return hit_fault;
}
