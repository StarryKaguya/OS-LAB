#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include<ctime>
#define SERVERNAME "HobushServer"

using namespace std;

const int MAX_STR_LENGTH = 10000, MAX_ROW = 1000;
vector<string> target;
int idx = 0;
bool IsLetter(char tar);
bool FindTarget(char dst[], char src[]);

void sig_chld(int signo)
{
	pid_t pid;
	int stat;

	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("child %d terminated\n", pid);
	return;
}
int main(int argc, char *argv[])
{
	cout << "start !" << endl;
    time_t t;
    t = time(NULL);
	char fileName[MAX_STR_LENGTH];		   //目标文件名
	char interestWordName[MAX_STR_LENGTH]; //要搜索的关键字
	if (argc != 3)
	{
		printf("Wrong Parameters Format,Exit Program");
		exit(0);
	}
	else
	{
		strcpy(fileName, argv[1]);
		strcpy(interestWordName, argv[2]);
	}

	struct sockaddr_un server, client; //本地sa类型

	server.sun_family = AF_UNIX; //指定为本地进程间通信
	strcpy(server.sun_path, SERVERNAME);

	int serverfd, clientfd;	  //文件描述符
	char buf[MAX_STR_LENGTH]; //用来数据通信的数组
	char buf2[2];

	pid_t pid = fork();
	if (pid < 0)
	{
		printf("Error in Fork");
		fprintf(stderr, "Fork Failed");
		return 0;
	}
	FILE *fp = NULL;

	if (pid > 0)
	{
		fp = fopen(fileName, "r");
		if (fp == NULL)
		{
			cout << "Open File Failed, Exit Program" << endl;
			fprintf(stderr, "Read File Failed");
			exit(0);
		}
		serverfd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (serverfd < 0)
		{
			printf("error in socket\n");
			exit(0);
		}
		unlink(SERVERNAME); //删除socket文件
		if (bind(serverfd, (struct sockaddr *)&server, sizeof(server)) < 0)
		{
			printf("error in bind\n");
			return 0;
		}
		listen(serverfd, 5);
		signal(SIGCHLD, sig_chld);
		socklen_t socket_len = sizeof(client);
		clientfd = accept(serverfd, (struct sockaddr *)&client, &socket_len);
		if (clientfd < 0)
		{
			close(clientfd);
			close(serverfd);
			printf("error clientfd\n");
			return 0;
		}
		//printf("test,id=%d\n",pid);
		while (1)
		{
			fgets(buf, MAX_STR_LENGTH, fp);
			if (feof(fp))
			{
				close(clientfd);
				close(serverfd);
				printf("Read File end!\n");
				break;
			}
			string tmp = buf;
			write(clientfd, buf, strlen(buf) + 1);
			read(clientfd, buf2, 2);
			if (buf2[0] == '1')
			{
				target.push_back(tmp);
			}
		}
		unlink(SERVERNAME);
		sort(target.begin(), target.end());
		FILE *fp2 = fopen("dest02.txt", "w");
		for (int i = 0; i < target.size(); i++)
		{
			printf("%s", target[i].c_str());
			fputs(target[i].c_str(), fp2);
		}
		fclose(fp2);
		cout << "end!"<<endl;
        double cost_t = time(NULL) - t;
        cout << "cost time : " << cost_t  << "s" << endl;
	}
	else
	{
		struct sockaddr_un client;
		int fd;
		fd = socket(AF_UNIX, SOCK_STREAM, 0);
		client.sun_family = AF_UNIX;
		strcpy(client.sun_path, SERVERNAME);
		bind(fd, (struct sockaddr *)&client, sizeof(client));
		int result = 0;
		result = connect(fd, (struct sockaddr *)&client, sizeof(client));
		if (result < 0)
		{
			printf("error in connect\n");
			return 0;
		}
		while (1)
		{
			int len = read(fd, buf, MAX_STR_LENGTH);
			char sendFlag[] = {'1', '\0'};
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
				write(fd, sendFlag, 2);
			}
			else if (len == 0)
			{
				cout << "Read Pipe Finished" << endl;
				close(fd);
				return 0;
			}
			else
			{
				close(fd);
				cout << "Read Pipe Failed" << endl;
				fprintf(stderr, "Read Pipe Failed");
				return 0;
			}
		}
	}
	return 0;
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