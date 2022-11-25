#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <vector>
#include <string>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <iostream>
#include <algorithm>
#include <pthread.h>
#include <ctime>
using namespace std;

const int MAX_STR_LENGTH = 10000, MAX_ROW = 1000;
vector<string> target;
int idx = 0;
bool IsLetter(char tar);
bool FindTarget(char dst[], char src[]);
void *FindForThread(void *param);

void sig_chld(int signo)
{
	pid_t pid;
	int stat;

	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("child %d terminated\n", pid);
	return;
}
union semun
{
	int val;			   /* Value for SETVAL */
	struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
	unsigned short *array; /* Array for GETALL, SETALL */
	struct seminfo *__buf; /* Buffer for IPC_INFO
							  (Linux-specific) */
};
struct range
{
	char *begin;
	char *end;
	char *interestName;
};
struct validRes
{
	char *str[MAX_ROW]; //合法行句子
	int size;			//包含的合法行个数
};
void P(int semid, unsigned short num)
{
	struct sembuf set;
	set.sem_num = num;		//信号量在数组里的序号
	set.sem_op = -1;		//信号量的操作值
	set.sem_flg = SEM_UNDO; //信号量的操作标识
	semop(semid, &set, 1);
	printf("get key %d——FromHobushP操作\n", (int)num);
}

void V(int semid, unsigned short num)
{
	struct sembuf set;
	set.sem_num = num;
	set.sem_op = 1;
	set.sem_flg = SEM_UNDO;
	semop(semid, &set, 1);
	printf("put key %d ——FromHobushV操作\n", (int)num);
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
	key_t key1, key2;
	key1 = ftok("hobush_sh", 1); //获取共享内存键值
	key2 = ftok("hobush_se", 2); //获取信号量键值

	int shmid = shmget(key1, 1024 * 1024 * 7, IPC_CREAT | 0666); //打开或者创建共享内存，因为文件最大就是6.5M，设置共享内存最大为7M
	int semid = semget(key2, 2, IPC_CREAT | 0666);				 //打开或者创建信号量组

	union semun seminit; //信号量初始化
	seminit.val = 1;	 //第一个信号量设置为1
	semctl(semid, 0, SETVAL, seminit);
	seminit.val = 0; //第二个信号量设置为0
	semctl(semid, 1, SETVAL, seminit);

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
		P(semid, 0);								//给第一个信号量上锁
		char *shmaddr = (char *)shmat(shmid, 0, 0); //共享内存连接到当前进程的地址空间
		int *num = (int *)shmaddr;
		char *tmp = shmaddr + 4;
		printf("Server shmat ok\n");
		// strcpy(shmaddr, "hello world"); //向内存中写入数据
		while (1)
		{
			*tmp = fgetc(fp);
			if (*tmp == '\n')
			{
				*num++;
			}
			tmp = tmp + 1;
			if (feof(fp))
			{
				break;
			}
		}
		//*tmp == '\0';
		fclose(fp);
		V(semid, 1); //释放第二个信号量

		P(semid, 0); //等待第一个信号量的释放
		num = (int *)shmaddr;
		int sum = *num;
		printf("sum=%d\n",sum);
		tmp = shmaddr + 4;
		for (int i = 0; i < sum; i++)
		{
			char *str1 = (char*)malloc(MAX_STR_LENGTH);//此类形式一定要防止
			//strcpy(str1,tmp);
			//int l = strlen(tmp);
			int i2 = 0;
			while (*tmp != '\0')
			{
				str1[i2++] = *tmp;
				tmp = tmp + 1;
			}
			str1[i2] = '\0';tmp = tmp + 1;
			if(str1=="\n"||str1=="\0") continue;
			target.push_back(str1);
		}
		
		sort(target.begin(), target.end());
		FILE *fp2 = fopen("dest03.txt", "w");
		for (int i = 0; i < target.size(); i++)
		{
			printf("%s", target[i].c_str());
			fputs(target[i].c_str(), fp2);
		}
		fclose(fp2);
		shmdt(shmaddr);				//断开进程和内存的连接
		shmctl(shmid, IPC_RMID, 0); //删除共享内存段
		semctl(semid, 0, IPC_RMID); //删除信号量组
		printf("quit-fromServer\n");
		signal(SIGCHLD, sig_chld);
		cout << "end!" << endl;
		double cost_t = time(NULL) - t;
		cout << "cost time : " << cost_t << "s" << endl;
		return 0;
	}
	else
	{
		P(semid, 1);								//等待第二个信号量释放
		char *shmaddr = (char *)shmat(shmid, 0, 0); //共享内存连接到当前进程的地址空间
		int *num = (int *)shmaddr;
		char *tmp = shmaddr + 4;
		printf("Client shmat ok\n"); //表示连接成功
		// printf("In client data : %s\n", tmp);	//将内存地址中的数据读出，打印
		int len = strlen(tmp);
		range range1 = {tmp, tmp + len / 4, interestWordName};
		range range2 = {tmp + len / 4 + 1, tmp + len / 2, interestWordName};
		range range3 = {tmp + len / 2 + 1, tmp + len * 3 / 4, interestWordName};
		range range4 = {tmp + len * 3 / 4 + 1, tmp + len - 1, interestWordName};
		pthread_t tid[4];
		pthread_create(&tid[0], NULL, FindForThread, &range1);
		pthread_create(&tid[1], NULL, FindForThread, &range2);
		pthread_create(&tid[2], NULL, FindForThread, &range3);
		pthread_create(&tid[3], NULL, FindForThread, &range4);
		validRes *res1 = (validRes *)malloc(sizeof(validRes));
		validRes *res2 = (validRes *)malloc(sizeof(validRes));
		validRes *res3 = (validRes *)malloc(sizeof(validRes));
		validRes *res4 = (validRes *)malloc(sizeof(validRes));
		pthread_join(tid[0], (void **)&res1);
		pthread_join(tid[1], (void **)&res2);
		pthread_join(tid[2], (void **)&res3);
		pthread_join(tid[3], (void **)&res4);
		// sort(target.begin(),target.end());
		int sum = 0;
		tmp = shmaddr + 4;
		for (int i = 0; i < res1->size; i++)
		{
			int l = strlen(res1->str[i]);
			sum++;
			strcpy(tmp, res1->str[i]);
			tmp = tmp + l + 1;
		}
		for (int i = 0; i < res2->size; i++)
		{
			int l = strlen(res2->str[i]);
			sum++;
			strcpy(tmp, res2->str[i]);
			tmp = tmp + l + 1;
		}
		for (int i = 0; i < res3->size; i++)
		{
			int l = strlen(res3->str[i]);
			sum++;
			strcpy(tmp, res3->str[i]);
			tmp = tmp + l + 1;
		}
		for (int i = 0; i < res4->size; i++)
		{
			int l = strlen(res4->str[i]);
			sum++;
			strcpy(tmp, res4->str[i]);
			tmp = tmp + l + 1;
		}
		*num = sum;
		/*FILE *fp2 = fopen("dest03.txt", "w");
		for (int i = 0; i < target.size(); i++)
		{
			printf("%s", target[i].c_str());
			fputs(target[i].c_str(), fp2);
		}
		fclose(fp2);*/
		shmdt(shmaddr); //断开内存和当前进程的连接
		V(semid, 0);	//释放第一个信号量
		printf("quit-FromClient\n");
		while (1)
			;
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
void *FindForThread(void *param)
{
	validRes *tmp = (validRes *)malloc(sizeof(validRes));
	range *tmp2 = (range *)param;
	int cnt = 0;
	char *begin = tmp2->begin;
	char *end = tmp2->end;
	char *name = tmp2->interestName;
	while (begin < end) {
		char *tmp3=(char*)malloc(MAX_STR_LENGTH);
		int idx = 0;
		char *t = begin;

		while (*t != '\n' && t < end) {
			tmp3[idx++] = *t;
			t = t + 1;
		}

		begin = t + 1;
		tmp3[idx] = '\n';
		tmp3[idx + 1] = '\0';

		if (FindTarget(name, tmp3)) {
			tmp->str[tmp->size++] = tmp3;//指向的是栈空间就出错
			//target.push_back(tmp3);
		}
	}
	return tmp;
}