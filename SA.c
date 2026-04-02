#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int n2=0;

void BubbleSort(int v[], int n){
    int i,j;
    for(i=0;i<n;i++){
        for(j=0;j<n;j++){
            if(v[i] < v[j]){
                int aux = v[i];
                v[i] = v[j];
                v[j] = aux;
            }
        }
    }
}

void BubbleSortDesc(int v[], int n){
    int i,j;
    for(i=0;i<n;i++){
        for(j=0;j<n;j++){ 
            if(v[i] > v[j]){
                int aux = v[i];
                v[i] = v[j];
                v[j] = aux;
            }
        }
    }
}

void MergeSort(int v[], int temp[],int s, int d){
    const int lungime = d-s+1;
    if(s==d){

        return;
    }else{
        int m = (s+d)/2;
        MergeSort(v,temp,s,m);
        MergeSort(v,temp,m+1,d);

        int i=s,j=m+1,k=0;

        while(i<=m && j <=d){
            if(v[i] > v[j]){
                temp[k] = v[j];
                j++;
            }else{
                temp[k] = v[i];
                i++;
            }
            k++;
        }

        while(i<=m){
            temp[k] = v[i];
            k++;i++;
        }
        while(j<=d){
            temp[k] = v[j];
            k++;j++;
        }
        for(int t = 0; t<k;t++){
            v[s+t] = temp[t];
        }
    }

}

void MergeSortTemp(int v[], int n){
    int *temp = malloc(n*sizeof(int));
    MergeSort(v,temp,0,n-1);
    free(temp);
}

void InsertionSort(int v[], int n){
    int i,aux,j;
    for(i=1;i<n;i++){
        aux = v[i];
        j=i-1;
        while(j>=0 && aux < v[j]){
            v[j+1] = v[j];
            j--;
        }
        v[j+1] = aux;
    }
}

void SelectionSort(int v[], int n){
    int i, min_index,j,aux;
    for(i=0;i<n;i++){
        min_index = i;
        for(j=i+1;j<n;j++){
            if(v[j] < v[min_index]){
                min_index = j;
            }
        }
        aux = v[min_index];
        v[min_index] = v[i];
        v[i] = aux;
    }
}

double measure_time(void (*alg)(int [], int), int v[], int n, int runs){
    int *copy = malloc(n*sizeof(int));
    clock_t total = 0;
    if (!copy){
        printf("Eroare la alocarea de memorie!");
        return -1;
    }
    for(int r =0;r < runs; r++){
        memcpy(copy,v,n*sizeof(int));
        clock_t start = clock();
        alg(copy,n);
        clock_t end = clock();
        total += end-start;
    }

    free(copy);

    return (double)1000.0 *(total)/CLOCKS_PER_SEC / runs;
}

void CountingSort(int v[], int n){
    int i, max = v[0];

    for(i = 1; i < n; i++){
        if(v[i] > max){
            max = v[i];
        }
    }

    int *count= (int*)malloc((max+1)*sizeof(int));

    for(i = 0; i <= max; i++){
        count[i] = 0;
    }

    for(i = 0; i < n; i++){
        count[v[i]]++;
    }

    int k = 0;
    for(i = 0; i <= max; i++){
        while(count[i] > 0){
            v[k] = i;
            k++;
            count[i]--;
        }
    }
    free(count);
}

void scrieInFisier(const char *numeFisier, int v[], int n){
    FILE *f = fopen(numeFisier, "w");
    if(f == NULL){
        printf("Eroare la deschiderea fisierului!\n");
        return;
    }

    fprintf(f, "%d\n", n);   
    for(int i = 0; i < n; i++){
        fprintf(f, "%d ", v[i]);
    }

    fclose(f);
}

int main() {
    int v[10000],v2[10000],v3[10000],v4[10000],v5[10000],n=10000;
    int *vm = malloc(1000000 * sizeof(int));
    if(vm == NULL){
        printf("Eroare la alocare\n");
        return 1;
    }
    for(int i=0;i<1000000;i++){
        vm[i] = i;
    }
    printf("Bubble sort %.6f",measure_time(BubbleSort,vm,1000000,1));
    free(vm);
 
    for(int i=0;i<n;i++){
        v[i] = rand();
        v2[i] = v[i];
        v3[i] = v[i];
        v4[i] = v[i];
        v5[i] = v[i];
    }


    /*/
    for(int i=0;i<n;i++){
        v[i] = rand();
    }
    BubbleSortDesc(v,n);
    for(int i=0;i<n;i++){
        v2[i] = v[i];
        v3[i] = v[i];
        v4[i] = v[i];
        v5[i] = v[i];
    }
    

    scrieInFisier("Date.txt",v,n);
    MergeSortTemp(v,n);
    scrieInFisier("DateCrescatoare.txt",v,n);
    MergeSortTemp(v3,n-100);
    scrieInFisier("AproapeCrescatoare.txt",v3,n);
    BubbleSortDesc(v2,n);
    scrieInFisier("DateDescrescatoare.txt",v2,n);
    for(int i = 0; i < n; i++){
        v3[i] = 100 + rand() % 5;   
    }
    scrieInFisier("DatePlate.txt",v3,n);
    /*/

    printf("TABLOU GENERAT!\n");

    double merge_ms = measure_time(MergeSortTemp, v, n, 1000);
    double bubble_ms = measure_time(BubbleSort, v2, n, 10);

    double insertion_ms = measure_time(InsertionSort, v, n, 1000);
    double selection_ms = measure_time(SelectionSort, v2, n, 1000);

    double counting_ms = measure_time(CountingSort, v, n, 1000);

    printf("Merge Sort: %.6f ms\n", merge_ms);
    printf("Merge Sort: %.6f s\n", merge_ms / 1000.0);
    printf("Bubble Sort: %.6f ms\n", bubble_ms);
    printf("Bubble Sort: %.6f s\n", bubble_ms / 1000.0);

    printf("Selection Sort: %.6f ms\n", selection_ms);
    printf("Selection Sort: %.6f s\n", selection_ms / 1000.0);
    printf("Insertion Sort: %.6f ms\n", insertion_ms);
    printf("Insertion Sort: %.6f s\n", insertion_ms / 1000.0);

    printf("Counting Sort: %.6f ms\n", counting_ms);
    printf("Counting Sort: %.6f s\n", counting_ms / 1000.0);
    return 0;
}