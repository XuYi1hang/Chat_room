#include <stdio.h>
#include <stdlib.h>
#include <network.h>
#include <pthread.h>

void* run(void *arg)
{
	char recv_buf[4096] = {};
	size_t buf_size = sizeof(recv_buf);
	NetWork* nw = arg;
	for(;;)
	{
		int ret = recv_nw(nw,recv_buf,buf_size);
		if(0 >= ret)
		{
			puts("服务器正在升级!");
			exit(EXIT_FAILURE);
		}
		printf("\r%s\n>>>",recv_buf);
		fflush(stdout);
	}
}

int main(int argc,const char* argv[])
{
	if(3!= argc)
	{
		puts("User: ./client <ip> <port>");
		return EXIT_SUCCESS;
	}

	NetWork* nw = init_nw(SOCK_STREAM,atoi(argv[2]),argv[1],false);
	if(NULL == nw)
	{
		puts("网络有误,请检查!");
		return EXIT_FAILURE;
	}
	
	char send_buf[4096] = {};
	size_t buf_size = sizeof(send_buf);

	printf("请输入你的昵称:");
	scanf("%s",send_buf);

	send_nw(nw,send_buf,strlen(send_buf)+1);

	//	开启接受群聊消息线程
	pthread_t tid;
	pthread_create(&tid,NULL,run,nw);

	for(;;)
	{
		printf(">>>");
		scanf("%s",send_buf);
		int ret = send_nw(nw,send_buf,strlen(send_buf)+1);
		if(0 == strcmp(send_buf,"quit"))
		{
			puts("已退出当前聊天室!");
			return EXIT_SUCCESS;
		}
		if(0 >= ret)
		{
			puts("服务器正在升级,请稍候!");
			return EXIT_FAILURE;
		}
	}
}
