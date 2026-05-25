#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

void merge(int *arr, int *tmp, int left, int mid, int right) {
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) {
            tmp[k++] = arr[i++];
        } else {
            tmp[k++] = arr[j++];
        }
    }

    while (i <= mid) {
        tmp[k++] = arr[i++];
    }

    while (j <= right) {
        tmp[k++] = arr[j++];
    }

    for (i = left; i <= right; i++) {
        arr[i] = tmp[i];
    }
}

void parallel_k_way_merge(int *arr, int *tmp, int left, int right, int chunk_size) {
    if (right - left + 1 <= chunk_size) {
        return;
    }

    int mid = left + (right - left) / 2;

    #pragma omp task shared(arr, tmp)
    parallel_k_way_merge(arr, tmp, left, mid, chunk_size);

    #pragma omp task shared(arr, tmp)
    parallel_k_way_merge(arr, tmp, mid + 1, right, chunk_size);

    #pragma omp taskwait

    merge(arr, tmp, left, mid, right);
}

int compare(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

int main() {
    int N = 16000000;
    int K = 16;
    int chunk_size = N / K;
    
    int *arr = (int*)malloc(N * sizeof(int));
    int *tmp = (int*)malloc(N * sizeof(int));

    srand(time(NULL));
    for(int i = 0; i < N; i++) {
        arr[i] = rand() % 1000000;
    }

    double start_time = omp_get_wtime();

    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < K; i++) {
                #pragma omp task
                qsort(arr + i * chunk_size, chunk_size, sizeof(int), compare);
            }
            #pragma omp taskwait
            
            parallel_k_way_merge(arr, tmp, 0, N - 1, chunk_size);
        }
    }

    double end_time = omp_get_wtime();

    int ok = 1;
    for(int i = 1; i < N; i++) {
        if(arr[i-1] > arr[i]) {
            ok = 0;
            break;
        }
    }

    if(ok) {
        printf("Merge Tree concluido com sucesso!\n");
        printf("Total de elementos: %d\n", N);
        printf("Tempo de execucao: %f segundos\n", end_time - start_time);
    } else {
        printf("Erro na ordenacao!\n");
    }

    free(arr);
    free(tmp);
    return 0;
}