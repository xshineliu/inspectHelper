#include <stdio.h>  
#include <stdlib.h>  
#include <cpuid.h>

#if __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40300
#define USE_CPUID_COUNT
#endif

typedef int boolean;


#define WORD_EAX  0
#define WORD_EBX  1
#define WORD_ECX  2
#define WORD_EDX  3
#define WORD_NUM  4

#define FALSE 0
#define TRUE 0

static int real_get (int           cpuid_fd,
                     unsigned int  reg,
                     unsigned int  words[],
                     unsigned int  ecx,
                     boolean       quiet)
{
#ifdef USE_CPUID_COUNT
      __cpuid_count(reg, ecx,
                    words[WORD_EAX],
                    words[WORD_EBX],
                    words[WORD_ECX],
                    words[WORD_EDX]);
#else
      asm("cpuid"
          : "=a" (words[WORD_EAX]),
            "=b" (words[WORD_EBX]),
            "=c" (words[WORD_ECX]),
            "=d" (words[WORD_EDX])
          : "a" (reg),
            "c" (ecx));
#endif
}
    
int t1() {
	char buf[64] = {0,};
	unsigned int l = 0x0;
	//unsigned int l = 0x40000000;
	//unsigned int l = 0x40000001;
	unsigned int *a = (unsigned int *)(buf + 0);
	unsigned int *b = (unsigned int *)(buf + 4);
	unsigned int *c = (unsigned int *)(buf + 8);
	unsigned int *d = (unsigned int *)(buf + 12);
	__cpuid_count(l, 0, a, b, c, d);
	//printf("%d %x %x %x %x\n", ret,  *a, *b, *c, *d);
	printf("%x %x %x %x\n",  *a, *b, *c, *d);
	//printf("%s\n", buf + 4);
	return 0;
}

int main() {

	unsigned int reg, max = 0;
	int            cpuid_fd   = -1;
	unsigned int  words[WORD_NUM];

	reg = 0x40000000;
	max = 0x40000000;
	real_get(cpuid_fd, reg, words, 0, FALSE);
        printf("%x/%x %x %x %x %x\n", reg, max, words[0], words[1], words[2], words[3]);	

	reg = 0;
	max = 0;
	for (reg = 0; reg <= max; reg++) {
		real_get(cpuid_fd, reg, words, 0, FALSE);
		printf("%d/%d %d %x %x %x\n", reg, max, words[0], words[1], words[2], words[3]);
		if (reg == 0) {
			max = words[WORD_EAX];
		}
	}

	reg = 0x40000000;
	max = 0x40000000;
	real_get(cpuid_fd, reg, words, 0, FALSE);
	printf("%x/%x %x %x %x %x\n", reg, max, words[0], words[1], words[2], words[3]);

	for (reg = 0x80000002; reg <= 0x80000004; reg++) {
		real_get(cpuid_fd, reg, words, 0, FALSE);
		printf("%x/%x %d %x %x %x\n", reg, max, words[0], words[1], words[2], words[3]);
	}

}
