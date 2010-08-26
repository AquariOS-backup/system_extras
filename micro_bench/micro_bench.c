/*
** Copyright 2010 The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/*
 * Some quick and dirty micro-benchmarks
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/time.h>

/* tv2 -= tv1 */
static void tv_sub(struct timeval *tv2, struct timeval *tv1) {
        tv2->tv_sec -= tv1->tv_sec;
        tv2->tv_usec -= tv1->tv_usec;
        while (tv2->tv_usec < 0) {
            tv2->tv_usec += 1000000;
            tv2->tv_sec -= 1;
        }
}

static int do_sleep(int delay) {
    struct timeval tv1;
    struct timeval tv2;

    while (1) {
        gettimeofday(&tv1, NULL);
        sleep(delay);
        gettimeofday(&tv2, NULL);

        tv_sub(&tv2, &tv1);

        printf("sleep(%d) took %ld.%06ld seconds\n", delay, tv2.tv_sec, tv2.tv_usec);
    }

    return 0;
}

int cpu_foo;

static int do_cpu(int a) {
    struct timeval tv1;
    struct timeval tv2;

    while (1) {
        gettimeofday(&tv1, NULL);
        for (cpu_foo = 0; cpu_foo < 100000000; cpu_foo++);
        gettimeofday(&tv2, NULL);

        tv_sub(&tv2, &tv1);

        printf("cpu took %ld.%06ld seconds\n", tv2.tv_sec, tv2.tv_usec);
    }
    return 0;
}

static double mb_sec(unsigned long bytes, struct timeval *delta) {
    unsigned long us = delta->tv_sec * 1000000 + delta->tv_usec;
    return (double)bytes * 1000000.0 / 1048576.0 / (double)us;
}

static int do_memset(int sz) {
    struct timeval tv1;
    struct timeval tv2;
    int i;

    uint8_t *b = malloc(sz);
    if (!b) return -1;
    int c = 1000000000/sz;

    while (1) {
        gettimeofday(&tv1, NULL);
        for (i = 0; i < c; i++)
            memset(b, 0, sz);

        gettimeofday(&tv2, NULL);

        tv_sub(&tv2, &tv1);

        printf("memset %dx%d bytes took %ld.%06ld seconds (%f MB/s)\n", c, sz, tv2.tv_sec, tv2.tv_usec, mb_sec(c*sz, &tv2));
    }
    return 0;
}

static int do_memcpy(int sz) {
    struct timeval tv1;
    struct timeval tv2;
    int i;

    uint8_t *a = malloc(sz);
    if (!a) return -1;
    uint8_t *b = malloc(sz);
    if (!b) return -1;
    int c = 1000000000/sz;

    while (1) {
        gettimeofday(&tv1, NULL);
        for (i = 0; i < c; i++)
            memcpy(b, a, sz);

        gettimeofday(&tv2, NULL);

        tv_sub(&tv2, &tv1);

        printf("memcpy %dx%d bytes took %ld.%06ld seconds (%f MB/s)\n", c, sz, tv2.tv_sec, tv2.tv_usec, mb_sec(c*sz, &tv2));
    }
    return 0;
}

int foo;

static int do_memread(int sz) {
    struct timeval tv1;
    struct timeval tv2;
    int i, j;

    int *b = malloc(sz);
    if (!b) return -1;
    int c = 1000000000/sz;

    while (1) {
        gettimeofday(&tv1, NULL);
        for (i = 0; i < c; i++)
            for (j = 0; j < sz/4; j++)
                foo = b[j];

        gettimeofday(&tv2, NULL);

        tv_sub(&tv2, &tv1);

        printf("read %dx%d bytes took %ld.%06ld seconds (%f MB/s)\n", c, sz, tv2.tv_sec, tv2.tv_usec, mb_sec(c*sz, &tv2));
    }
    return 0;
}

struct {
    char *name;
    int (*ptr)(int);
} function_table[]  = {
    {"sleep", do_sleep},
    {"cpu", do_cpu},
    {"memset", do_memset},
    {"memcpy", do_memcpy},
    {"memread", do_memread},
    {NULL, NULL},
};

static void usage() {
    int i;

    printf("Usage:\n");
    for (i = 0; function_table[i].name; i++) {
        printf("\tmicro_bench %s ARG\n", function_table[i].name);
    }
}

int main(int argc, char **argv) {
    int i;

    if (argc != 3) {
        usage();
        return -1;
    }
    for (i = 0; function_table[i].name; i++) {
        if (!strcmp(argv[1], function_table[i].name)) {
            printf("%s\n", function_table[i].name);
            return (*function_table[i].ptr)(atoi(argv[2]));
        }
    }
    usage();
    return -1;
}
