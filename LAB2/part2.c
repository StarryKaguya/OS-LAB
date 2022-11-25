#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
const int ARRAYLENGtH = 10;
int arr[10] = {7, 12, 19, 3, 18, 4, 2, 6, 15, 8};
int sortArray[10]; //默认全为0
typedef struct
{
    int start;
    int end; // not included
} parameters;

void *Sort(void *param)
{
    parameters *temp = (parameters *)param;
    int t;
    memcpy(sortArray+temp->start, arr+temp->start, sizeof(int)*(temp->end-temp->start+1));
    int i = 0, j = 0;
    for (i = temp->start; i <= temp->end; i++)
    {
        for (j = temp->start; j <= temp->end - 1; j++)
        {
            if (sortArray[j] > sortArray[j + 1])
            {
                t = sortArray[j];
                sortArray[j] = sortArray[j + 1];
                sortArray[j + 1] = t;
            }
        }
    }
    return 0;
}
void *Merge(void *param)
{
    parameters *temp = (parameters *)param;
    int tmp[10];
    memcpy(tmp,sortArray,sizeof(sortArray));
    int mid = (temp->start+temp->end)/2;
    int i = temp->start, j = mid + 1;
    int k = temp->start;
    while(i <= mid && j < temp->end){
        if(tmp[i] <= tmp[j]){
            sortArray[k++] = tmp[i++];
        }
        else {
            sortArray[k++] = tmp[j++];
        }
    }
    while(i <= mid)
        sortArray[k++] = tmp[i++];
    while(j < temp->end)
        sortArray[k++] = tmp[j++];
    return 0;
}
int main(void)
{
    pthread_t tid_Left;
    pthread_t tid_Right;
    pthread_t tid_Merge;

    int mid = ARRAYLENGtH / 2;

    parameters *param_Left = (parameters *)malloc(sizeof(parameters));
    param_Left->start = 0;
    param_Left->end = mid;
    pthread_create(&tid_Left, NULL, Sort, param_Left);

    parameters *param_Right = (parameters *)malloc(sizeof(parameters));
    param_Right->start = mid + 1;
    param_Right->end = ARRAYLENGtH - 1;
    pthread_create(&tid_Right, NULL, Sort, param_Right);

    //堵塞直到两个排序线程完成
    pthread_join(tid_Left, NULL);
    pthread_join(tid_Right, NULL);
    parameters *param_Merge = (parameters *)malloc(sizeof(parameters));
    param_Merge->start = 0;
    param_Merge->end = ARRAYLENGtH;
    pthread_create(&tid_Merge, NULL, Merge, param_Merge);
    pthread_join(tid_Merge, NULL);
    for(int i = 0; i < ARRAYLENGtH; i++){
        printf("%d ",sortArray[i]);
    }
    printf("\n");
}