#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <vector>
#include <string>
#include<ctime>
using namespace std;
const int MAX_STR_LENGTH = 10000, MAX_ROW = 1000;
vector<string> target;
int idx = 0;
bool FindTargetDemo(char dst[], char src[])
{
    char *ret = strstr(src, dst);
    if (ret == NULL)
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool IsLetter(char tar)
{
    if (tar <= 'z' && tar >= 'a')
        return true;
    if (tar <= 'Z' && tar >= 'A')
        return true;
    return false;
}
bool FindTarget(char dst[], char src[])
{
    char *ret = strstr(src, dst);
    if (ret == NULL)
    {
        return false;
    }
    else
    {
        int len = strlen(dst);
        if (ret == src && (*(ret + len) == '\0' || *(ret + len) == '\n'))
            return true;
        if (ret == src && !IsLetter(*(ret + len)))
        {
            return true;
        }
        else if ((*(ret + len) == '\0' || *(ret + len) == '\n') && !IsLetter(*(ret - 1)))
        {
            return true;
        }
        else if (!IsLetter(*(ret + len)) && !IsLetter(*(ret - 1)))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}
int main(int argc, char *argv[])
{

    char fileName[MAX_STR_LENGTH];         //目标文件名
    char interestWordName[MAX_STR_LENGTH]; //要搜索的关键字
    cout << "start !" << endl;
    time_t t;
    t = time(NULL);
    if (argc != 3)
    {
        cout << "Wrong Parameters Format,Exit Program" << endl;
        exit(0);
    }
    else
    {
        strcpy(fileName, argv[1]);
        strcpy(interestWordName, argv[2]);
    }

    int fd[2], fd2[2];
    if (pipe(fd) == -1)
    { //父进程写，子进程
        cout << "Error in Creating Pipe" << endl;
        fprintf(stderr, "Pipe Failed");
        return 1;
    }
    if (pipe(fd2) == -1)
    { //父进程读，子进程写
        cout << "Error in Creating Pipe2" << endl;
        fprintf(stderr, "Pipe2 Failed");
        return 1;
    }

    pid_t pid = fork(); // FAP
    if (pid < 0)
    {
        cout << "Error in Fork" << endl;
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else
    {
        FILE *fp = NULL;
        if (pid > 0)
        {
            close(fd[0]);  //父进程中关闭pipe1读端
            close(fd2[1]); //父进程中关闭pipe2写端
            fp = fopen(fileName, "r");
            if (fp == NULL)
            {
                cout << "Open File Failed, Exit Program" << endl;
                fprintf(stderr, "Read File Failed");
                exit(0);
            }
        }
        else
        {
            close(fd[1]);
            close(fd2[0]);
        }
        while (1) //子进程无法退出循环，不会成为僵尸进程，成为孤儿进程后会被Init进程处理
        {
            if (pid > 0) //父进程
            {
                char buf[MAX_STR_LENGTH];
                fgets(buf, MAX_STR_LENGTH, fp);//在一个足够大的常数下读到的都有换行符
                if (feof(fp))
                {
                    fclose(fp);
                    close(fd[1]);  //读取文件完毕，父进程中关闭pipe1写端
                    close(fd2[0]); //读取文件完毕，父进程中关闭pipe2读端
                    break;
                }
                string tmp = buf;
                write(fd[1], buf, strlen(buf)+1);
                read(fd2[0], buf, MAX_STR_LENGTH);
                if (buf[0] == '1')
                {
                    //printf("tmp=%s",tmp.c_str());
                   // printf("id=%d\n",idx++);
                    target.push_back(tmp);
                }
            }
            else
            {
                char buf[MAX_STR_LENGTH];
                char sendFlag[] = {'1', '\0'};
                int len = read(fd[0], buf, MAX_STR_LENGTH);
                if (len > 0) //大于0代表读取成功，不成功有0（读到末尾）和-1（系统调用失败）
                {
                    bool flag = true;
                    if (buf[0] == '\n' || buf[0] == '\0')
                        flag = false;
                    else
                        flag = FindTarget(interestWordName, buf);
                    if (!flag)
                    {
                        sendFlag[0] = '\0';
                    }
                    write(fd2[1], sendFlag, sizeof(sendFlag));
                }
                else if (len == 0)
                {
                    cout << "Read Pipe Finished" << endl;
                    close(fd[0]);
                    close(fd2[1]);
                    return 0;
                }
                else
                {
                    close(fd[0]);
                    close(fd2[1]);
                    cout << "Read Pipe Failed" << endl;
                    fprintf(stderr, "Read Pipe Failed");
                    return 0;
                }
            }
        }
        sort(target.begin(), target.end());
        FILE *fp2 = fopen("dest01.txt", "w");
        for (int i = 0; i < target.size(); i++)
        {
            printf("%s", target[i].c_str());
            fputs(target[i].c_str(), fp2);
        }
        fclose(fp2);
        cout << "end!"<<endl;
        double cost_t = time(NULL) - t;
        cout << "cost time : " << cost_t  << "s" << endl;
        return 0;
        
    }
}