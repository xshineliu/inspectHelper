/*
* threadinfo.c
*
*  Created on: Aug 12, 2015
*      Author: Shine
*/


#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#define MAX_LEN 4096

extern int errno;


float get_uptime(void)
{
        FILE *f = NULL;
        char *chk = NULL;
        char buf[MAX_LEN];
        float uptime = 0.0f;

        f = fopen("/proc/uptime", "r");
        if(!f) {
                printf("No such file?\n");
                return errno;
        }

        chk = fgets(buf, MAX_LEN, f);
        if(!chk) {
                printf("Error to Read.\n");
        }

        uptime = atof(chk);

        //printf("uptime:\t%.2f\n", uptime);
        fclose(f);
        return uptime;
}


void print_comm(pid_t pid, pid_t tid)
{
        FILE *f = NULL;
        char *chk = NULL;
        char filePath[64];
        char buf[MAX_LEN];

        sprintf(filePath, "/proc/%d/task/%d/comm", pid, tid);
        f = fopen(filePath, "r");
        if(!f) {
                fprintf(stderr, "No such file?\n");
                return;
        }

        while((chk = fgets(buf, MAX_LEN, f))) {
        }
	buf[strlen(buf) - 1] = '\0';
        printf("%s\t", buf);
        fclose(f);
}


int print_stat(pid_t pid, pid_t tid)
{
        FILE *f = NULL;
        char *chk = NULL;
        char filePath[64];
        char buf[MAX_LEN];
        int i = 0;
        float val;
        sprintf(filePath, "/proc/%d/task/%d/stat", pid, tid);
        f = fopen(filePath, "r");
        if(!f) {
                printf("No such file?\n");
                return errno;
        }
        chk = fgets(buf, MAX_LEN, f);
        if(!chk) {
                printf("Error to Read.\n");
        }
        //printf("%s\n", chk);
        while (i < 13) {
                chk++;
                if((*chk) == ' ') {
                        i++;
                }
        }
        /* item 14 */
        val = (float)(atoi(++chk)) / 100.f;
        printf("User \t%.2f\t", val);
        while((*chk) != ' ') {
                chk++;
        }
        i++;

        /* item 15 */
        val = (float)(atoi(++chk)) / 100.f;
        printf("Sys\t%.2f\t", val);
        i++;

        while (i < 22) {
                chk++;
                if((*chk) == ' ') {
                        i++;
                }
        }
        //printf("%s\n", chk);
        /* item 22 */
        val = (float)(atoi(++chk)) / 100.f;
        printf("Wall\t%.2f\t", get_uptime() - val);

        fclose(f);
}


int print_status(pid_t pid, pid_t tid)
{
        FILE *f = NULL;
        char *chk = NULL;
        char filePath[64];
        char buf[MAX_LEN];
        int i = 0;
        float val;
        long long v_ctx = 0L, nonv_ctx = 0L;

        sprintf(filePath, "/proc/%d/task/%d/status", pid, tid);
        f = fopen(filePath, "r");
        if(!f) {
                printf("No such file?\n");
                return errno;
        }

        while((chk = fgets(buf, MAX_LEN, f))) {
                // at least 3 chars without '\0'
                if(chk[0] == 'v' && chk[1] == 'o' && chk[2] == 'l') {
                        while(*chk != '\t') {
                                chk++;
                        }
                        chk++;
                        //printf("%s", chk);
                        v_ctx = atol(chk);
                        continue;
                }
                if(chk[0] == 'n' && chk[1] == 'o' && chk[2] == 'n') {
                        i = 0;
                        while(*chk != '\t') {
                                chk++;
                        }
                        chk++;
                        //printf("%s", chk);
                        nonv_ctx = atol(chk);
                        continue;
                }
        }
        printf("CTX\t%lld %lld\t", v_ctx, nonv_ctx);
        fclose(f);
}

int dump_info(int pid, int tid){

        printf("PID %d TID %d\t", pid, tid);
        print_comm(pid, tid);
        get_uptime();
        print_stat(pid, tid);
        print_status(pid, tid);
        printf("\n");
}

int dump_info_pid_only(int pid) {
        DIR* dir;
        struct dirent *entry;
	char path_buf[32];
	int tid;

	sprintf(path_buf, "/proc/%d/task", pid);
	if((dir = opendir(path_buf)) == NULL) {
		fprintf(stderr, "PID %d not valid or open proc directory failed\n", pid);
		exit(errno);
	}

	while ( ( entry = readdir ( dir ) ) ){
                if ( strcmp( entry->d_name, "." ) && strcmp( entry->d_name, ".." )){
	                //printf("/proc/%d/tasks/%s\n", pid, entry->d_name);
			tid = atoi(entry->d_name);
			dump_info(pid, tid);
		}
         }
        closedir(dir);
}

#ifndef THREADINFO_AS_LIB

int main(int argc, char* argv[])
{
        pid_t pid = 0;
        pid_t tid = 0;
	DIR* dir;
	struct dirent *entry;
	char path_buf[32];

	if(argc < 2) {
        	fprintf(stderr, "Usage: %s PID [TID]\n", argv[0]);
		exit(1);
	}

        pid = atoi(argv[1]);
	if(argc > 2) {
        	tid = atoi(argv[2]);
		dump_info(pid, tid);
		exit(1);
	}


	dump_info_pid_only(pid);
        return 0;
}

#endif
