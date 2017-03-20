/*
 *
 * ***** Orignal from libpfm4. See the copyright below.
 *
 * syst.c - example of a simple system wide monitoring program
 *
 * Copyright (c) 2002-2006 Hewlett-Packard Development Company, L.P.
 * Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <err.h>
#include <sys/time.h>

#include "perf_util.h"

typedef struct {
	const char *events;
	int repeats;
	int excl;
	int cpu;
	int group;
	int verbose;
	int interval;
} options_t;

static options_t options;
static perf_event_desc_t **all_fds;
static int *num_fds;
static int fd;

static struct timeval time_stamp;

void
setup_cpu(int cpu)
{
	perf_event_desc_t *fds;
	int i, ret;

	ret = perf_setup_list_events(options.events, &all_fds[cpu], &num_fds[cpu]);
	if (ret || (num_fds == 0))
		errx(1, "cannot setup events\n");
	fds = all_fds[cpu]; /* temp */

	fds[0].fd = -1;
	for(i=0; i < num_fds[cpu]; i++) {
		fds[i].hw.disabled = options.group ? !i : 1;

		if (options.excl && ((options.group && !i) || (!options.group)))
			fds[i].hw.exclusive = 1;
			
		fds[i].hw.disabled = options.group ? !i : 1;

		/* request timing information necessary for scaling counts */
		fds[i].hw.read_format = PERF_FORMAT_SCALE;
		fds[i].fd = perf_event_open(&fds[i].hw, -1, cpu, (options.group ? fds[0].fd : -1), 0);
		if (fds[i].fd == -1)
			err(1, "cannot attach event to CPU%d %s", cpu, fds[i].name);
	}
}

void
measure(void)
{
	perf_event_desc_t *fds;
	long lret;
	int c, cmin, cmax, ncpus;
	int i, ret, l;
	uint64_t tempu64;
	//uint64_t *last_vec;

	printf("<press CTRL-C to quit before %ds time limit>\n", options.repeats);

	cmin = 0;

	lret = sysconf(_SC_NPROCESSORS_ONLN);
	if (lret < 0)
		err(1, "cannot get number of online processors");

	cmax = (int)lret;

	ncpus = cmax;
	if (options.cpu != -1) {
		cmin = options.cpu;
		cmax = cmin + 1;
	}
	all_fds = calloc(ncpus, sizeof(perf_event_desc_t *));
	num_fds = calloc(ncpus, sizeof(int));

	if (!all_fds || !num_fds)
		err(1, "cannot allocate memory for internal structures");
	for(c=cmin ; c < cmax; c++)
		setup_cpu(c);

	/*
	 * FIX this for hotplug CPU
	 */
	for(c=cmin ; c < cmax; c++) {
		fds = all_fds[c];
		if (options.group) 
			ret = ioctl(fds[0].fd, PERF_EVENT_IOC_ENABLE, 0);
		else for(i=0; i < num_fds[c]; i++) {
			ret = ioctl(fds[i].fd, PERF_EVENT_IOC_ENABLE, 0);
			if (ret)
				err(1, "cannot enable event %s\n", fds[i].name);
		}
	}

	//last_vec = calloc(ncpus * num_fds[0], sizeof(uint64_t));
	tempu64 = 0x0123456789ABCDEFLL;
	write(fd, &tempu64, sizeof(tempu64));
	tempu64 = (uint64_t) (cmax - cmin)  + ((uint64_t) num_fds[cmin] << 32);
	write(fd, &tempu64, sizeof(tempu64));
	tempu64 = (uint64_t) options.interval / 1000;
	write(fd, &tempu64, sizeof(tempu64));
	tempu64 = (uint64_t) options.repeats;
	write(fd, &tempu64, sizeof(tempu64));


	for(l=0; l < options.repeats; l++) {

		usleep(options.interval);

		gettimeofday (&time_stamp, NULL);
		write(fd, &(time_stamp.tv_sec), sizeof(time_stamp.tv_sec));

		// for each CPU
		for(c = cmin; c < cmax; c++) {
			fds = all_fds[c];

			// for each event
			for(i=0; i < num_fds[c]; i++) {
				uint64_t val, delta;
				double ratio;

				ret = read(fds[i].fd, fds[i].values, sizeof(fds[i].values));
				if (ret != sizeof(fds[i].values)) {
					if (ret == -1)
						err(1, "cannot read event %d:%d", i, ret);
					else
						warnx("could not read event%d", i);
				}

				/*
				 * scaling because we may be sharing the PMU and
				 * thus may be multiplexed
				 */
				delta = perf_scale_delta(fds[i].values, fds[i].prev_values);
				if(options.verbose == 1) {
					val = perf_scale(fds[i].values);
					ratio = perf_scale_ratio(fds[i].values);
					fprintf(stderr, "CPU%d val=%-20"PRIu64" %-20"PRIu64" raw=%"PRIu64" ena=%"PRIu64" run=%"PRIu64" ratio=%.2f %s\n",
						c,
						val,
						delta,
						fds[i].values[0],
						fds[i].values[1], fds[i].values[2], ratio,
						fds[i].name);
				}
				fds[i].prev_values[0] = fds[i].values[0];
				fds[i].prev_values[1] = fds[i].values[1];
				fds[i].prev_values[2] = fds[i].values[2];
				write(fd, &delta, sizeof(delta));
			}
		}

		fprintf(stdout, "---\t%u\t%lu.%lu\t---\n", l + 1, time_stamp.tv_sec, time_stamp.tv_usec);
	}

	for(c = cmin; c < cmax; c++) {
		fds = all_fds[c];
		for(i=0; i < num_fds[c]; i++)
			close(fds[i].fd);
		perf_free_fds(fds, num_fds[c]);
	}
}

static void usage(char* s)
{
	printf("usage: %s [-c cpu] [-x] [-h] [-d duration per repeat in ms] [-n repeats] [-g] [-v] [-e event1,event2,...]\n", s);
}

int
main(int argc, char **argv)
{
	int c, ret;
	char *filename = "pmu_count.bin";
	options.cpu = -1;
	options.verbose = 0;
	options.interval = 998 * 1000;

	while ((c=getopt(argc, argv,"hc:e:n:d:gxv")) != -1) {
		switch(c) {
			case 'x':
				options.excl = 1;
				break;
			case 'e':
				options.events = optarg;
				break;
			case 'c':
				options.cpu = atoi(optarg);
				break;
			case 'g':
				options.group = 1;
				break;
			case 'v':
				options.verbose = 1;
				break;
			case 'd':
				options.interval = atoi(optarg) * 1000;
				break;
			case 'n':
				options.repeats = atoi(optarg);
				break;
			case 'h':
				usage(argv[0]);
				exit(0);
			default:
				errx(1, "unknown error");
		}
	}
	if (!options.repeats)
		options.repeats = 5;

	if (!options.events) {
		//options.events = "cycles,instructions";
		options.events = "cycles,instructions,MEM_UOPS_RETIRED:ALL_LOADS,MEM_UOPS_RETIRED:ALL_STORES,"
			"OFFCORE_RESPONSE_0:SNP_ANY:L3_MISS_LOCAL:DMND_DATA_RD,OFFCORE_RESPONSE_0:SNP_ANY:L3_MISS_LOCAL:PF_DATA_RD,"
			"OFFCORE_RESPONSE_0:SNP_ANY:L3_MISS_LOCAL:DMND_RFO,OFFCORE_RESPONSE_0:SNP_ANY:L3_MISS_LOCAL:PF_RFO,"
			"OFFCORE_RESPONSE_1:SNP_ANY:L3_MISS_REMOTE:DMND_DATA_RD,OFFCORE_RESPONSE_1:SNP_ANY:L3_MISS_REMOTE:PF_DATA_RD,"
			"OFFCORE_RESPONSE_1:SNP_ANY:L3_MISS_REMOTE:DMND_RFO,OFFCORE_RESPONSE_1:SNP_ANY:L3_MISS_REMOTE:PF_RFO";
	}

	if((fd = creat(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)  {
		errx(1, "Output Create Error");
		exit(1);
	}


	ret = pfm_initialize();
	if (ret != PFM_SUCCESS)
		errx(1, "libpfm initialization failed: %s\n", pfm_strerror(ret));
	
	measure();

	/* free libpfm resources cleanly */
	pfm_terminate();

	close(fd);

	return 0;
}
