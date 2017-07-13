#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>  //getpid()
#include <semaphore.h> 
#include <fcntl.h>
#include <errno.h>
#include <sys/syscall.h>   //gettid() thread
#include<time.h>
#include <string.h>      //for memset
//创建模式权限
#define FILE_MODE  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define BUFFER 5 // 缓冲区数量
#define PRO_NO 30 // PRODUCING NO
#define OVER ( - 1)
#define PSLEEP 10000  // 
#define CSLEEP 10000  // 

#define PRODUCER 15  // 生产者线程数量
#define CONSUMER 10  // 消费者线程数量

//#define SEM_MUTEX   "lock"  /* 信号量lock用于对缓冲区的互斥操作 */
//#define SEM_NEMPTY  "notempty" /* 缓冲区非空的信号量 */
//#define SEM_NFULL   "notfull" /* 缓冲区未满的信号量 */

    //int PRODUCER;//生产者数量
    //int CONSUMER;//消费者数量
    //int BUFFER;//缓冲区数量
    //int ONCE;//生产者一次生产的个数
#define gettid() syscall(SYS_gettid)   //获取线程ID
    sem_t empty,full;//同步信号量
    pthread_mutex_t mutex;//互斥信号量
    //int *buffer; //缓冲区
struct prodcons
{// 缓冲区相关数据结构
	int buf[BUFFER]; /* 实际数据存放的数组*/
	int tid[BUFFER];			//生产进程PID   getpid() 头文件
	char *time[BUFFER];			//写入时间
	int readpos, writepos; /* 读写指针*/
};
struct prodcons buffer;

/* 初始化缓冲区结构 */
void init(struct prodcons *b)
{
	b->readpos = 0;
	b->writepos = 0;
}
int producer_id=0,consumer_id=0;//生产者消费者ID
//int index_in=0,index_out=0;//生产者 消费者 存放 消费的位置

void *Producer()//生产者函数
{
	//int thread_arg;
//thread_arg = *(int *)data;
int n=0;
time_t timep;
time(&timep);
char *tem;
int nn=PRO_NO;
   while(nn--)
   {

	 sleep(3);
	 sem_wait(&empty);
	 pthread_mutex_lock(&mutex);
	 
	buffer.buf[buffer.writepos] = n;
	buffer.tid[buffer.writepos] = gettid();  //threadid
	tem = asctime(gmtime(&timep));
	buffer.time[buffer.writepos] = tem; //time
	printf("data:%d ,tid: %ld ,time_in: %s --->\n", n,gettid(),tem);
		
	buffer.writepos++;
	if (buffer.writepos >= BUFFER)
		buffer.writepos = 0;
	 pthread_mutex_unlock(&mutex);
	 sem_post(&full);
	 n++;
   }
   
}
void *Consumer()//消费者函数
{
	int d;
int gettid;
char *tt;
   while(1)
   {
	 sleep(3);
	 sem_wait(&full);
	 pthread_mutex_lock(&mutex);
	/* 读数据,移动读指针*/
	d = buffer.buf[buffer.readpos];
	gettid = buffer.tid[buffer.readpos];
	tt = buffer.time[buffer.readpos];
	//usleep(CSLEEP);
	buffer.readpos++;
	if (buffer.readpos >= BUFFER)
		buffer.readpos = 0;
	printf("--->data:%d ,tid: %d ,time_in: %s \n", d,gettid,tt);
	if (d == OVER)
		break;
	 
	 pthread_mutex_unlock(&mutex);
	 sem_post(&empty);
   }
}

int main()
{
init(&buffer);
   int rthread[18],i;
   pthread_t producer[PRODUCER];//生产者
   pthread_t consumer[CONSUMER];//消费者

   int sinit1=sem_init(&empty,0,BUFFER);//初始化同步信号量
   int sinit2=sem_init(&full,0,0);
   int minit =pthread_mutex_init(&mutex,NULL);//初始化互斥信号量
   if(sinit1 && sinit2)
   {
	 printf("sem initialize failed /n");
	 exit(1);
   }
   if(minit)
   {
	 printf("sem initialize failed /n");
	 exit(1);
   }
   for(i=0;i<PRODUCER;i++)//创建生产者线程
   {
	  rthread[i]=pthread_create(&producer[i], NULL, Producer, NULL);
	  if(rthread[i])
	  {
		  printf("producer %d create failed /n", i);
		  exit(1);
	  }
   }
   for(i=0;i<CONSUMER;i++)//创建消费者线程
   {
	  rthread[i]=pthread_create(&consumer[i], NULL, Consumer,NULL);
	  if(rthread[i])
	  {
		  printf("consumer %d create failed /n", i);
		  exit(1);
	  }
   }
   for(i=0;i<PRODUCER;i++)//销毁生产者线程
   {
	  pthread_join(producer[i],NULL);
   }
   for(i=0;i<CONSUMER;i++)//销毁生产者线程
   {
	  pthread_join(consumer[i],NULL);
   }
   exit(0);
}
