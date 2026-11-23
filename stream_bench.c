/*
 * stream_bench.c - STREAM-style memory bandwidth benchmark
 *
 * Usage:
 *   gcc -O2 -o stream_bench stream_bench.c
 *   ./stream_bench              # default: 16M doubles (128 MB/array)
 *   ./stream_bench 8388608      # 8M doubles (64 MB/array)
 *
 * Measures Copy / Scale / Add / Triad bandwidth (MB/s and GB/s).
 * Array size should be several times larger than LLC to be DRAM-bound.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

static volatile double sink;
static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char **argv)
{
    size_t n = (argc > 1) ? strtoull(argv[1], NULL, 10) : (16u * 1024u * 1024u);
    int iter = (argc > 2) ? atoi(argv[2]) : 60;
    int repeats = (argc > 3) ? atoi(argv[3]) : 5;
    const double scalar = 3.0;

    if (n == 0 || iter <= 0 || repeats <= 0) {
        fprintf(stderr, "usage: %s [num_doubles] [iterations] [repeats]\n", argv[0]);
        return 1;
    }

    size_t bytes = n * sizeof(double);
    double *a = aligned_alloc(64, bytes);
    double *b = aligned_alloc(64, bytes);
    double *c = aligned_alloc(64, bytes);
    if (!a || !b || !c) {
        fprintf(stderr, "alloc failed (need ~%.0f MB); reduce array size\n",
                3.0 * bytes / 1048576.0);
        free(a); free(b); free(c);
        return 1;
    }
    for (size_t i = 0; i < n; i++) {
        a[i] = 1.0;
        b[i] = 2.0;
        c[i] = 0.0;
    }

    printf("stream_bench: %zu doubles (%.0f MB x 3 = %.0f MB), %d iters x %d repeats\n",
           n, bytes / 1048576.0, 3.0 * bytes / 1048576.0, iter, repeats);
    printf("bytes/iter: Copy/Scale=%zu, Add/Triad=%zu\n\n",
           2 * n * sizeof(double), 3 * n * sizeof(double));

    double t, best;

    /* Copy: c = a */
    best = 1e9;
    for (int k = 0; k < repeats; k++) {
        t = now();
        for (int i = 0; i < iter; i++)
            memcpy(c, a, bytes);
        t = now() - t;
        if (t < best) best = t;
    }
    printf("Copy  : %8.1f MB/s  (%6.2f GB/s)\n",
           2.0 * bytes * iter / best / 1e6, 2.0 * bytes * iter / best / 1e9);

    /* Scale: b = scalar * c */
    best = 1e9;
    for (int k = 0; k < repeats; k++) {
        t = now();
        for (int i = 0; i < iter; i++)
            for (size_t j = 0; j < n; j++)
                b[j] = scalar * c[j];
        t = now() - t;
        if (t < best) best = t;
    }
    sink = b[n - 1];
    printf("Scale : %8.1f MB/s  (%6.2f GB/s)\n",
           2.0 * bytes * iter / best / 1e6, 2.0 * bytes * iter / best / 1e9);

    /* Add: c = a + b */
    best = 1e9;
    for (int k = 0; k < repeats; k++) {
        t = now();
        for (int i = 0; i < iter; i++)
            for (size_t j = 0; j < n; j++)
                c[j] = a[j] + b[j];
        t = now() - t;
        if (t < best) best = t;
    }
    sink = c[n - 1];
    printf("Add   : %8.1f MB/s  (%6.2f GB/s)\n",
           3.0 * bytes * iter / best / 1e6, 3.0 * bytes * iter / best / 1e9);

    /* Triad: a = b + scalar * c */
    best = 1e9;
    for (int k = 0; k < repeats; k++) {
        t = now();
        for (int i = 0; i < iter; i++)
            for (size_t j = 0; j < n; j++)
                a[j] = b[j] + scalar * c[j];
        t = now() - t;
        if (t < best) best = t;
    }
    sink = a[n - 1];
    printf("Triad : %8.1f MB/s  (%6.2f GB/s)\n",
           3.0 * bytes * iter / best / 1e6, 3.0 * bytes * iter / best / 1e9);

    free(a); free(b); free(c);
    (void)sink;
    return 0;
}
