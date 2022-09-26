#include <stdio.h>
#include <stdlib.h>
#include <network.h>
#include <stdbool.h>
#include <threadpool.h>

#define MAX_CLIENT (50)
//	buf的字节数
#define BUF_SIZE (4096)

//	在线客户端nw的数组
NetWork* cli_arr[MAX_CLIENT];

//	添加nw
int add_clinw(NetWork* nw)
{
	for(int i=0; i<MAX_CLIENT; i++)
	{
		if(NULL == cli_arr[i])
		{
			cli_arr[i] = nw;	
			return i;
		}
	}
}

//	转发消息给其他客户端
void send_cli_arr(const char* msg,int index)
{
	for(int i=0; i<MAX_CLIENT; i++)
	{
		if(NULL != cli_arr[i] && i!= index)
		{
			send_nw(cli_arr[i],msg,strlen(msg)+1);	
		}
	}
}

//	消费者业务逻辑函数
void chat_work(void* arg)
{
	//	添加arg到客户端nw数组中
	int index =	add_clinw(arg);

	char* buf = malloc(BUF_SIZE);
	//	接收昵称
	int ret = recv_nw(cli_arr[index],buf,BUF_SIZE);
	if(0 >= ret || 0 == strcmp(buf,"quit"))
	{
		free(buf);
		close_nw(cli_arr[index]);
		cli_arr[index] = NULL;
		return;
	}
	
	strcat(buf," 进入聊天室,大家欢迎!");
	send_cli_arr(buf,index);

	buf[ret-1] = ':';
	char* msg = buf+ret;

	for(;;)
	{
		int	msg_ret = recv_nw(cli_arr[index],msg,BUF_SIZE-ret);
		if(0 >= msg_ret || 0 == strcmp(msg,"quit"))
		{
			sprintf(msg,"退出聊天室!");
			send_cli_arr(buf,index);
			free(buf);
			close_nw(cli_arr[index]);
			cli_arr[index] = NULL;
			return;
		}
		send_cli_arr(buf,index);
	}
}

int main(int argc,const char* argv[])
{
	if(3 != argc)
	{
		printf("User: ./server <ip> <port>\n");
		return EXIT_SUCCESS;
	}

	//	启动网络服务
	NetWork* svr_nw = init_nw(SOCK_STREAM,atoi(argv[2]),argv[1],true);
	if(NULL == svr_nw)
	{
		puts("网络错误,请检查!");
		return EXIT_FAILURE;
	}

	//	创建并启动线程池
	ThreadPool* threadpool = create_threadpool(MAX_CLIENT,20,chat_work);
	start_threadpool(threadpool);

	for(;;)
	{
		//	生产者 等待客户端连接
		NetWork* cli_nw = accept_nw(svr_nw);
		if(NULL == cli_nw)
		{
			puts("网络异常!");
			continue;
		}

		//	添加数据到仓库
		push_threadpool(threadpool,cli_nw);
	}
}
