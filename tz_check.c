// gcc -o tz_check tz_check.c -ldl -static

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include <stdbool.h>
#include <limits.h>


int nerrs = 0;

typedef long (*vgtod_t)(struct timeval *tv, struct timezone *tz);
vgtod_t vdso_gettimeofday;

static inline int sys_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	return syscall(__NR_gettimeofday, tv, tz);
}

static void fill_function_pointers()
{
	void *vdso = dlopen("linux-vdso.so.1",
			    RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD);
	if (!vdso)
		vdso = dlopen("linux-gate.so.1",
			      RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD);
	if (!vdso) {
		printf("[WARN]\tfailed to find vDSO\n");
		return;
	}


	vdso_gettimeofday = (vgtod_t)dlsym(vdso, "__vdso_gettimeofday");
	if (!vdso_gettimeofday)
		printf("Warning: failed to find gettimeofday in vDSO\n");

}

static bool ts_leq(const struct timespec *a, const struct timespec *b)
{
	if (a->tv_sec != b->tv_sec)
		return a->tv_sec < b->tv_sec;
	else
		return a->tv_nsec <= b->tv_nsec;
}

static bool tv_leq(const struct timeval *a, const struct timeval *b)
{
	if (a->tv_sec != b->tv_sec)
		return a->tv_sec < b->tv_sec;
	else
		return a->tv_usec <= b->tv_usec;
}

static void test_gettimeofday(void)
{
	struct timeval start, vdso, end;
	struct timezone sys_tz, vdso_tz;
	int vdso_ret, end_ret;

	if (!vdso_gettimeofday)
		return;

	printf("[RUN]\tTesting gettimeofday...\n");

	if (sys_gettimeofday(&start, &sys_tz) < 0) {
		printf("[FAIL]\tsys_gettimeofday failed (%d)\n", errno);
		nerrs++;
		return;
	}

	vdso_ret = vdso_gettimeofday(&vdso, &vdso_tz);
	end_ret = sys_gettimeofday(&end, NULL);

	if (vdso_ret != 0 || end_ret != 0) {
		printf("[FAIL]\tvDSO returned %d, syscall errno=%d\n",
		       vdso_ret, errno);
		nerrs++;
		return;
	}

	printf("\t%llu.%06ld %llu.%06ld %llu.%06ld\n",
	       (unsigned long long)start.tv_sec, start.tv_usec,
	       (unsigned long long)vdso.tv_sec, vdso.tv_usec,
	       (unsigned long long)end.tv_sec, end.tv_usec);

	if (!tv_leq(&start, &vdso) || !tv_leq(&vdso, &end)) {
		printf("[FAIL]\tTimes are out of sequence\n");
		nerrs++;
	}

	if (sys_tz.tz_minuteswest == vdso_tz.tz_minuteswest &&
	    sys_tz.tz_dsttime == vdso_tz.tz_dsttime) {
		printf("[OK]\ttimezones match: minuteswest=%d, dsttime=%d\n",
		       sys_tz.tz_minuteswest, sys_tz.tz_dsttime);
	} else {
		printf("[FAIL]\ttimezones do not match\n");
		nerrs++;
	}

	/* And make sure that passing NULL for tz doesn't crash. */
	vdso_gettimeofday(&vdso, NULL);
}

int main(int argc, char **argv)
{
	fill_function_pointers();

	test_gettimeofday();

	return nerrs ? 1 : 0;
}
