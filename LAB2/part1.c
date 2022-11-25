#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
void *ColumnCheck(void *param);
void *RowCheck(void *param);
void *SquareCheck(void *parameter);
int Sudoku[9][9] = {
    {6, 2, 4, 5, 3, 9, 1, 8, 7},
    {5, 1, 9, 7, 2, 8, 6, 3, 4},
    {8, 3, 7, 6, 1, 4, 2, 9, 5},
    {1, 4, 3, 8, 6, 5, 7, 2, 9},
    {9, 5, 8, 2, 4, 7, 3, 6, 1},
    {7, 6, 2, 3, 9, 1, 4, 5, 8},
    {3, 7, 1, 9, 5, 6, 8, 4, 2},
    {4, 9, 6, 1, 8, 2, 5, 7, 3},
    {2, 8, 5, 4, 7, 3, 9, 1, 6},
};

typedef struct
{
    int row;
    int column;
} parameters;

typedef struct{
    int flag;
}result;

int main(void)
{
    pthread_t tid_Column;
    pthread_t tid_Row;
    pthread_t tid_Square[3][3];
    //行检查
    pthread_create(&tid_Row, NULL, RowCheck, NULL);
    //列检查
    pthread_create(&tid_Column, NULL, ColumnCheck, NULL);
    //子方块检查
    parameters *param_square[3][3];
    for (int i = 0; i < 3; i++) //块行
    {
        for (int j = 0; j < 3; j++) //块列
        {
            param_square[i][j] = (parameters *)malloc(sizeof(parameters));
            param_square[i][j]->row = i * 3;
            param_square[i][j]->column = j * 3;
            pthread_create(&tid_Square[i][j], NULL, SquareCheck, param_square[i][j]);
        }
    }
    result *rowRes = (result*)malloc(sizeof(result));
    result *colRes = (result*)malloc(sizeof(result));
    pthread_join(tid_Column, (void**)&rowRes);
    pthread_join(tid_Row, (void**)&colRes);
    result *squareRes[3][3];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            squareRes[i][j] = (result*)malloc(sizeof(result));
            pthread_join(tid_Square[i][j], (void**)&squareRes[i][j]);
        }
    }
    int flag = 1; //判断数独是否有效，每个子线程通过检查相应的数据设置flag是否为0
    if(rowRes->flag != 1 || colRes->flag != 1){
        flag = 0;
    }
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            if(squareRes[i][j]->flag != 1) {
                flag = 0;
                break;
            }
        }
    }
    if(flag == 1){
        printf("Valid Sudoku !\n");
    }
    else {
        printf("Unvalid Sudoku!\n");
    }
    //释放程序申请的空间
    free(rowRes);
    free(colRes);
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            free(squareRes[i][j]);
    return 0;
}
void *RowCheck(void *param)
{   
    result *tmp = (result*)malloc(sizeof(result));
    for (int i = 0; i < 9; i++)
    {
        int arr[10] = {0};
        for (int j = 0; j < 9; j++)
        {
            int num = Sudoku[i][j];
            if (arr[num] == 0)
            {
                arr[num] = 1;
            }
            else
            { 
                tmp->flag = 0;
                return tmp;
                //pthread_exit(0);
            }
        }
    }
    tmp->flag = 1;
    return tmp;
    //pthread_exit(0);
}
void *ColumnCheck(void *param)
{
    result *tmp = (result*)malloc(sizeof(result));
    for (int i = 0; i < 9; i++)
    {
        int arr[10] = {0};
        for (int j = 0; j < 9; j++)
        {
            int num = Sudoku[j][i];
            if (arr[num] == 0)
            {
                arr[num] = 1;
            }
            else
            {
                tmp->flag = 0;
                return tmp;
            }
        }
    }
    tmp->flag = 1;
    return tmp;
}
void *SquareCheck(void *parameter)
{
    result *tmp = (result*)malloc(sizeof(result));
    parameters *param = (parameters *)parameter;
    int arr[10] = {0};
    for (int i = param->row; i < param->row + 3; i++)
    {
        for (int j = param->column; j < param->column + 3; j++)
        {
            int num = Sudoku[i][j];
            if (arr[num] == 0)
            {
                arr[num] = 1;
            }
            else
            {
                tmp->flag = 0;
                return tmp;
            }
        }
    }
    tmp->flag = 1;
    return tmp;
}