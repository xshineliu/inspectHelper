/*
* gcc -O2 -g  -o bad_fpu_test2 bad_fpu_test2.c -mavx2 -lpthread
*/

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

int infinite = 0;




__attribute__((noinline)) void test_intel_256(float a, float b, float c)
{
__m256 res,vec1,vec2,vec3;

vec1 = _mm256_set_ps(a, a, a, a, a, a, a, a);
vec2 = _mm256_set_ps(b, b, b, b, b, b, b, b);
vec3 = _mm256_set_ps(c, c, c, c, c, c, c, c);

res = _mm256_add_ps(vec1, vec2);
res = _mm256_sub_ps(res, vec3);

/*
if (res[0] ==0.0f && res[1] ==0.0f && res[2] ==0.0f && res[3] ==0.0f
  && res[4] ==0.0f && res[5] ==0.0f && res[6] ==0.0f && res[7] ==0.0f )
    printf("Addition : OK!\n");
else
    printf("Addition : FAILED!\n");
*/
}


__attribute__((noinline)) void test_func1(void) {

  __m128i res1;
  __m128i magic1;
  __m128i val1;

  unsigned int expector = 0xFFFFFFFF;
  unsigned int obtained = 0xFFFFFFFF;
  unsigned int cur_pattern = 0xFFFFFFFF;
  unsigned int last_pattern = 0xFFFFFFFF;
  size_t hit1 = 0;
  size_t hit2 = 0;

  for (; expector != 0; expector--) {

    magic1 = _mm_set_epi32(0x0, 0x0, 0x45300000, expector);
    val1 = _mm_set_epi32(0x0, 0x0, 0x0, 0x29);

   __asm__ __volatile__ ("vmovupd %1, %%xmm1\n\t"
        "vmovupd %2, %%xmm0\n\t"
        "punpckldq %%xmm0,%%xmm1\n\t"
  "__BRKH:\n\t"
        "vmovdqu %%xmm1, %0\n\t"
        : "=m"(res1)
        : "m"(val1), "m"(magic1)
        : "xmm0", "xmm1");

   obtained = (unsigned int)(res1[0] >> 32);
   if(obtained != expector) {
      hit1++;

      if( (obtained >> 24) != ((obtained & 0x00FF0000) >> 16) ) {
        hit2++;
      }

      cur_pattern = expector & 0x00FF0000;
      if ( cur_pattern != last_pattern) {
        last_pattern = cur_pattern;

        // only print an example cycle, EA short for example
        if ( (expector & 0xFF000000) == 0xEA000000 ){
          printf("0x%08lx 0x%08lx 0x%08lx\t", obtained ^ expector, hit1, hit2);
          printf("0x%016llx, 0x%016llx\t", magic1[0], magic1[1]);
          printf("0x%016llx, 0x%016llx\n", res1[0], res1[1]);
        }
      }
    }
  }

   printf("Total Hit : 0x%08lx 0x%08lx\n", hit1, hit2);
}


int main(int argc, char* argv[]) {
  int s = 0;
  float a = 3.0f;
  float b = 4.0f;
  float c = 0.0f;

  cpu_set_t cpuset;

  if(argc > 1) {
    infinite = atoi(argv[1]);
    float c = a + b;
  }

  CPU_ZERO(&cpuset);
  CPU_SET(infinite, &cpuset);

  pthread_t thread;
  thread = pthread_self();

  //if(aff >= total_online) {
  //   printf("Error thread_index = %d, total_online = %d\n", val, total_online);
  //   return NULL;
  //}

  s = pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);
  if (s != 0) {
      error(EXIT_FAILURE, s, "pthread_setaffinity_np %d", infinite);
  }

  //total_online = sysconf(_SC_NPROCESSORS_ONLN);
  //printf("Online %d of max %d, infinite config %d\n", total_online, N, infinite);
  //    error(EXIT_FAILURE, N, "please recompile the app");

  test_func1();
  test_intel_256(a, b, c);

  exit(EXIT_SUCCESS);
}
