#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 80 /* The maximum length command */
#define LIGHT_PURPLE "\033[1;35m"
#define END_LIGHT_PURPLE "\033[0m"
int ampersandFlag = 0;
void PrintHistory(char historyGroup[10][MAX_LINE], int historyCount)
{
    printf("Up to the previous 10 Shell Command History:\n");
    int num = historyCount < 10 ? historyCount : 10;
    int len = historyCount;
    for (int i = 0; i < num; i++)
    {
        printf("%d  ", len);
        fputs(historyGroup[i], stdout);
        printf("\n");
        len--;
    }
}
//提取命令以及命令的参数，存储在args字符串数组
char* DivideCommand(char *args[MAX_LINE],char *commandStr)
{
    int i = 0;
    char *token = strtok(commandStr," ");
    while (token != NULL)
    {
        if (strcmp(token, "&") == 0)
        {
            ampersandFlag = 1;
        }
        else
        {
            args[i++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL; // NULL代表参数已经存储完毕
}
int main(void)
{
    int should_run = 1;           /* flag to determine when to exit program */
    char historyGroup[10][MAX_LINE];/*store up to 10 ten command history */
    int historyCount = 0;
    while (should_run)
    {   char *args[MAX_LINE / 2 + 1]; /* command line arguments */
        int shiftFlag = 1;//历史命令数组是否移位的标志，是1代表有新命令位于数组首位，其他命令右移
        ampersandFlag = 0;

        printf(LIGHT_PURPLE); // 给本程序的命令标志加上独特颜色
        printf("osh> ");
        printf(END_LIGHT_PURPLE);

        fflush(stdout); // 刷新缓冲区
        char commandStr[MAX_LINE];
        fgets(commandStr, MAX_LINE, stdin); // 读命令

        if (commandStr[strlen(commandStr) - 1] == '\n') // handle of function fgets()
        {
            commandStr[strlen(commandStr) - 1] = '\0';//replace with '\0' 
        }

        char *token = DivideCommand(args,commandStr);//提取命令以及它的参数
        if (strcmp(token, "exit") == 0)        // 情况一：exit命令，直接退出程序
        {
            should_run = 0;
            continue;
        }
        if (strcmp(args[0], "History") == 0) //情况二：History命令
        {
            if (historyCount > 0)
            {
                PrintHistory(historyGroup, historyCount); // 调用展示历史命令的函数
            }
            else
            {
                printf("No Commands in the history!\n");
            }
            continue;
        }
        else if (strcmp(args[0], "!!") == 0)//情况三：!!命令，执行上一次执行的命令
        {
            if (historyCount > 0)
            {
                shiftFlag = 0;
                ampersandFlag = 0;
                strcpy(commandStr, historyGroup[0]);           
                token = DivideCommand(args, commandStr);
            }
            else
            {
                printf("No recent command in the history!");
                continue;
            }
        }
        else if (args[0][0] == '!')//情况四:!命令，执行指定下标的命令
        {   
            shiftFlag = 0;
            ampersandFlag = 0;
            if (args[0][1] == '\0')//代表只有一个"!"，没有数字，非法命令
            {
                printf("Please enter correct command.\n");
                continue;
            }
            int x = historyCount - atoi(args[0] + 1);//读取!后的数字
            if (x > 10 || x < 0)
            {
                printf("Please enter correct command.\n");
                continue;
            }  
            strcpy(commandStr, historyGroup[x]);
            printf("Command executed: %s\n", commandStr);
            token = DivideCommand(args, commandStr);
        }
        if (shiftFlag == 1)//有新命令，老命令右移，新命令位于historyGroup[0]
        {
            for (int i = 9; i > 0; i--) // Moving the previous history
                strcpy(historyGroup[i], historyGroup[i - 1]);
            strcpy(historyGroup[0], "");
            for (int i = 0; args[i] != NULL; i++)
            {
                strcat(historyGroup[0], args[i]);
                strcat(historyGroup[0], " ");
            }
            historyCount++;
        }
        int pid = fork();//克隆一个子进程执行args中存储的shell命令
        if (pid == 0)
        {
            if(args + 1 != NULL)
            if (execvp(args[0], args) < 0)
            {
                printf("Please enter valid command. \n");
                return -1;
            }
            exit(0);//结束子进程
        }
        else
        {
            if (ampersandFlag == 0)//没有&符号，需要父进程自我堵塞
            {
                while (wait(NULL) != pid)//返回的不是fork()的子进程ID，继续堵塞直到正确的子进程死亡
                    ;
            }
        }
        fflush(stdin);
        /**
         * After reading user input, the steps are:
         * (1) fork a child process using fork()
         * (2) the child process will invoke execvp()
         * (3) if command included &, parent will invoke wait()
         */
    }
    return 0;
}