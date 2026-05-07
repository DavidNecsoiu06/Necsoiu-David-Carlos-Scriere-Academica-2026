#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _OPENMP
#include <omp.h>
#endif

#define DEFAULT_N 10000
#define DEFAULT_RUNS 20
#define DEFAULT_MAX_VALUE 10000
#define NEARLY_SORTED_PERCENT 1
#define PARALLEL_THRESHOLD 50000
#define PARALLEL_MAX_DEPTH 4

typedef void (*SortFunction)(int *v, int n);

typedef struct {
    const char *name;
    SortFunction function;
} Algorithm;

typedef enum {
    DATA_RANDOM = 0,
    DATA_SORTED,
    DATA_REVERSED,
    DATA_NEARLY_SORTED,
    DATA_FLAT,
    DATASET_COUNT
} DatasetType;

const char *dataset_name(DatasetType type) {
    switch (type) {
        case DATA_RANDOM: return "aleator";
        case DATA_SORTED: return "sortat_crescator";
        case DATA_REVERSED: return "sortat_descrescator";
        case DATA_NEARLY_SORTED: return "aproape_sortat";
        case DATA_FLAT: return "liste_plate";
        default: return "necunoscut";
    }
}

double now_seconds(void) {
#ifdef _WIN32
    static LARGE_INTEGER frequency;
    LARGE_INTEGER counter;

    if (frequency.QuadPart == 0) {
        QueryPerformanceFrequency(&frequency);
    }

    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)frequency.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
#endif
}

void swap_int(int *a, int *b) {
    int aux = *a;
    *a = *b;
    *b = aux;
}

int is_sorted_ascending(const int *v, int n) {
    for (int i = 1; i < n; i++) {
        if (v[i - 1] > v[i]) {
            return 0;
        }
    }
    return 1;
}

void generate_dataset(int *v, int n, int max_value, DatasetType type, unsigned int seed) {
    srand(seed + (unsigned int)type * 1009u);

    if (type == DATA_RANDOM) {
        for (int i = 0; i < n; i++) {
            v[i] = rand() % (max_value + 1);
        }
    } else if (type == DATA_SORTED) {
        for (int i = 0; i < n; i++) {
            v[i] = i;
        }
    } else if (type == DATA_REVERSED) {
        for (int i = 0; i < n; i++) {
            v[i] = n - i;
        }
    } else if (type == DATA_NEARLY_SORTED) {
        for (int i = 0; i < n; i++) {
            v[i] = i;
        }

        int swaps = n * NEARLY_SORTED_PERCENT / 100;
        if (swaps < 1 && n > 1) {
            swaps = 1;
        }

        for (int i = 0; i < swaps; i++) {
            int a = rand() % n;
            int b = rand() % n;
            swap_int(&v[a], &v[b]);
        }
    } else if (type == DATA_FLAT) {
        for (int i = 0; i < n; i++) {
            v[i] = rand() % 5;
        }
    }
}

void bubble_sort(int *v, int n) {
    for (int i = 0; i < n - 1; i++) {
        int changed = 0;

        for (int j = 0; j < n - i - 1; j++) {
            if (v[j] > v[j + 1]) {
                swap_int(&v[j], &v[j + 1]);
                changed = 1;
            }
        }

        if (!changed) {
            break;
        }
    }
}

void insertion_sort(int *v, int n) {
    for (int i = 1; i < n; i++) {
        int key = v[i];
        int j = i - 1;

        while (j >= 0 && v[j] > key) {
            v[j + 1] = v[j];
            j--;
        }

        v[j + 1] = key;
    }
}

void selection_sort(int *v, int n) {
    for (int i = 0; i < n - 1; i++) {
        int min_index = i;

        for (int j = i + 1; j < n; j++) {
            if (v[j] < v[min_index]) {
                min_index = j;
            }
        }

        if (min_index != i) {
            swap_int(&v[i], &v[min_index]);
        }
    }
}

void merge_ranges(int *v, int *temp, int left, int mid, int right) {
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right) {
        if (v[i] <= v[j]) {
            temp[k++] = v[i++];
        } else {
            temp[k++] = v[j++];
        }
    }

    while (i <= mid) {
        temp[k++] = v[i++];
    }

    while (j <= right) {
        temp[k++] = v[j++];
    }

    for (i = left; i <= right; i++) {
        v[i] = temp[i];
    }
}

void merge_sort_rec(int *v, int *temp, int left, int right) {
    if (left >= right) {
        return;
    }

    int mid = left + (right - left) / 2;
    merge_sort_rec(v, temp, left, mid);
    merge_sort_rec(v, temp, mid + 1, right);
    merge_ranges(v, temp, left, mid, right);
}

void merge_sort(int *v, int n) {
    if (n <= 1) {
        return;
    }

    int *temp = (int *)malloc((size_t)n * sizeof(int));
    if (temp == NULL) {
        fprintf(stderr, "Eroare: nu s-a putut aloca vectorul temporar pentru Merge Sort.\n");
        exit(EXIT_FAILURE);
    }

    merge_sort_rec(v, temp, 0, n - 1);
    free(temp);
}

#ifdef _OPENMP
void merge_sort_parallel_rec(int *v, int *temp, int left, int right, int depth) {
    if (left >= right) {
        return;
    }

    int mid = left + (right - left) / 2;
    int length = right - left + 1;

    if (length < PARALLEL_THRESHOLD || depth >= PARALLEL_MAX_DEPTH) {
        merge_sort_rec(v, temp, left, mid);
        merge_sort_rec(v, temp, mid + 1, right);
    } else {
        #pragma omp task shared(v, temp)
        merge_sort_parallel_rec(v, temp, left, mid, depth + 1);

        #pragma omp task shared(v, temp)
        merge_sort_parallel_rec(v, temp, mid + 1, right, depth + 1);

        #pragma omp taskwait
    }

    merge_ranges(v, temp, left, mid, right);
}

void merge_sort_parallel(int *v, int n) {
    if (n <= 1) {
        return;
    }

    int *temp = (int *)malloc((size_t)n * sizeof(int));
    if (temp == NULL) {
        fprintf(stderr, "Eroare: nu s-a putut aloca vectorul temporar pentru Merge Sort paralel.\n");
        exit(EXIT_FAILURE);
    }

    #pragma omp parallel
    {
        #pragma omp single
        merge_sort_parallel_rec(v, temp, 0, n - 1, 0);
    }

    free(temp);
}
#endif

void counting_sort(int *v, int n) {
    if (n <= 1) {
        return;
    }

    int min_value = v[0];
    int max_value = v[0];

    for (int i = 1; i < n; i++) {
        if (v[i] < min_value) {
            min_value = v[i];
        }
        if (v[i] > max_value) {
            max_value = v[i];
        }
    }

    long long range = (long long)max_value - (long long)min_value + 1;
    if (range <= 0 || range > 10000000LL) {
        fprintf(stderr, "Eroare: interval prea mare pentru Counting Sort.\n");
        exit(EXIT_FAILURE);
    }

    int *count = (int *)calloc((size_t)range, sizeof(int));
    if (count == NULL) {
        fprintf(stderr, "Eroare: nu s-a putut aloca vectorul de frecventa.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        count[v[i] - min_value]++;
    }

    int k = 0;
    for (int i = 0; i < range; i++) {
        while (count[i] > 0) {
            v[k++] = i + min_value;
            count[i]--;
        }
    }

    free(count);
}

void copy_array(int *destination, const int *source, int n) {
    memcpy(destination, source, (size_t)n * sizeof(int));
}

double measure_average_ms(SortFunction function, const int *base, int n, int runs,
                          double *min_ms, double *max_ms, int *all_sorted) {
    int *work = (int *)malloc((size_t)n * sizeof(int));
    if (work == NULL) {
        fprintf(stderr, "Eroare: nu s-a putut aloca vectorul de lucru.\n");
        exit(EXIT_FAILURE);
    }

    double total_ms = 0.0;
    *min_ms = 1e100;
    *max_ms = 0.0;
    *all_sorted = 1;

    for (int run = 0; run < runs; run++) {
        copy_array(work, base, n);

        double start = now_seconds();
        function(work, n);
        double finish = now_seconds();

        double elapsed_ms = (finish - start) * 1000.0;
        total_ms += elapsed_ms;

        if (elapsed_ms < *min_ms) {
            *min_ms = elapsed_ms;
        }
        if (elapsed_ms > *max_ms) {
            *max_ms = elapsed_ms;
        }
        if (!is_sorted_ascending(work, n)) {
            *all_sorted = 0;
        }
    }

    free(work);
    return total_ms / (double)runs;
}

int main(int argc, char **argv) {
    int n = DEFAULT_N;
    int runs = DEFAULT_RUNS;
    int max_value = DEFAULT_MAX_VALUE;

    if (argc >= 2) {
        n = atoi(argv[1]);
    }
    if (argc >= 3) {
        runs = atoi(argv[2]);
    }
    if (argc >= 4) {
        max_value = atoi(argv[3]);
    }

    if (n <= 0 || runs <= 0 || max_value <= 0) {
        fprintf(stderr, "Utilizare: %s [n] [runs] [max_value]\n", argv[0]);
        return EXIT_FAILURE;
    }

    Algorithm algorithms[] = {
        {"Bubble Sort optimizat", bubble_sort},
        {"Insertion Sort", insertion_sort},
        {"Selection Sort", selection_sort},
        {"Merge Sort", merge_sort},
#ifdef _OPENMP
        {"Merge Sort paralel OpenMP", merge_sort_parallel},
#endif
        {"Counting Sort", counting_sort}
    };

    int algorithm_count = (int)(sizeof(algorithms) / sizeof(algorithms[0]));

    int *base = (int *)malloc((size_t)n * sizeof(int));
    if (base == NULL) {
        fprintf(stderr, "Eroare: nu s-a putut aloca vectorul initial.\n");
        return EXIT_FAILURE;
    }

    FILE *csv = fopen("results.csv", "w");
    if (csv == NULL) {
        fprintf(stderr, "Eroare: nu se poate crea results.csv.\n");
        free(base);
        return EXIT_FAILURE;
    }

    fprintf(csv, "dataset,n,algorithm,runs,avg_ms,min_ms,max_ms,sorted_ok\n");

    printf("n = %d, runs = %d, max_value = %d\n", n, runs, max_value);
#ifdef _OPENMP
    printf("OpenMP activ: %d thread-uri maxime\n", omp_get_max_threads());
#else
    printf("OpenMP inactiv: compileaza cu -fopenmp pentru varianta paralela.\n");
#endif
    printf("Rezultatele sunt salvate in results.csv\n\n");

    for (int dataset = 0; dataset < DATASET_COUNT; dataset++) {
        generate_dataset(base, n, max_value, (DatasetType)dataset, 42u);

        for (int a = 0; a < algorithm_count; a++) {
            double min_ms, max_ms;
            int sorted_ok;
            double avg_ms = measure_average_ms(algorithms[a].function, base, n, runs,
                                               &min_ms, &max_ms, &sorted_ok);

            printf("%-18s | %-26s | avg = %10.4f ms | ok = %s\n",
                   dataset_name((DatasetType)dataset),
                   algorithms[a].name,
                   avg_ms,
                   sorted_ok ? "da" : "nu");

            fprintf(csv, "%s,%d,%s,%d,%.6f,%.6f,%.6f,%s\n",
                    dataset_name((DatasetType)dataset), n, algorithms[a].name, runs,
                    avg_ms, min_ms, max_ms, sorted_ok ? "yes" : "no");
        }
    }

    fclose(csv);
    free(base);

    return EXIT_SUCCESS;
}
