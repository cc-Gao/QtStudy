#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/un.h>

#define MAX_CONNECT_NUM 32
#define BUFSIZE 1024
#define ALIASLENGTH 32
#define PORT 7890

void accept_connect(int lfd, int events, void* arg);
void handle_data(int fd, int events, void* arg);


typedef struct eventInfo
{
	int fd;
	int events;
	void* arg;
	void (*call_back)(int fd, int events, void* arg);
	int status;
	char buf[BUFSIZE];
	int len;
	char alias[ALIASLENGTH];
}eventInfo;

int g_efd;
eventInfo g_events_info[MAX_CONNECT_NUM + 1];


void init_event_info(eventInfo* ev, int fd, void(*call_back)(int, int, void*), void* arg)
{
	ev->fd = fd;
	ev->call_back = call_back;
	ev->events = 0;					//仅仅做一个初始化
	ev->arg = arg;					//指向自己结构体的首地址（函数的最后一个参数）
	ev->status = 0;
	return;
}

void del_event_info(int efd, eventInfo* ev)
{
	struct epoll_event epv = { 0,{0} };

	if (ev->status != 1)
		return;

	epv.data.ptr = ev;
	ev->status = 0;
	epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv);

	return;
}

void handle_event_info(int efd, int events, eventInfo* ev)
{
	struct epoll_event epv = { 0,{0} };
	int op;
	epv.data.ptr = ev;
	epv.events = ev->events = events;

	if (ev->status == 1)										//已经在树上
	{
		op = EPOLL_CTL_MOD;
	}
	else
	{
		op = EPOLL_CTL_ADD;
		ev->status = 1;
	}

	if (epoll_ctl(efd, op, ev->fd, &epv) < 0)
		printf("event handle faild!\tfd=%d,events:%d\n", ev->fd, events);
	else
		printf("event handle success!\tfd=%d,events:%d\n", ev->fd, events);

	return;
}

void init_socket(int efd)
{
	int lfd = socket(AF_INET, SOCK_STREAM, 0);

	fcntl(lfd, F_SETFL, O_NONBLOCK);					//设置成非阻塞

	if (lfd < 0)
	{
		printf("create socket error \n");
		exit(1);
	}
	//初始化事件
	//void init_event(eventInfo *ev,int fd,void ((*call_back)(int ,int ,void *),void *arg);
	init_event_info(&g_events_info[MAX_CONNECT_NUM], lfd, accept_connect, &g_events_info[MAX_CONNECT_NUM]);		//将g_events最后赋值为lfd，并将其与accept_connect()绑定

	//将事件添加进efd
	//void handle_event(int efd,int events,eventInfo *ev)
	handle_event_info(efd, EPOLLIN, &g_events_info[MAX_CONNECT_NUM]);
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));


	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	//绑定
	int bret = bind(lfd, (struct sockaddr*) & server_addr, sizeof(server_addr));
	if (bret < 0)
	{
		printf("bind error\n");
		exit(1);
	}

	listen(lfd, 6);
	return;
}

void accept_connect(int lfd, int events, void* arg)
{
	struct sockaddr_in client_addr;
	socklen_t addr_size = sizeof(client_addr);

	int i;
	int cfd = accept(lfd, (struct sockaddr*) & client_addr, &addr_size);
	if (cfd == -1)
	{
		if (errno != EAGAIN && errno != EINTR)
		{
			//处理
		}
		printf("accept,%s\n", strerror(errno));
	}

	do		//为该cfd配置相关属性
	{
		for (i = 0; i < MAX_CONNECT_NUM; i++)
			if (g_events_info[i].status == 0)
				break;
		if (i == MAX_CONNECT_NUM)
		{
			printf("connect limit\n");
			break;
		}

		int flag = 0;
		flag = fcntl(cfd, F_SETFL, O_NONBLOCK);
		if (flag < 0)
		{
			printf("fcntl nonblocking error!\n");
			break;
		}


		//给cfd设置其对应的eventInfo
		init_event_info(&g_events_info[i], cfd, handle_data, &g_events_info[i]);
		handle_event_info(g_efd, EPOLLIN, &g_events_info[i]);
		
		printf("new connect:%d\n", cfd);

	} while (0);

	return;
}

void handle_data(int fd, int events, void* arg)
{
	eventInfo* ev = (eventInfo*)arg;
	ssize_t len;
	len = recv(fd, ev->buf, sizeof(ev->buf), 0);
	if (len > 0)
	{
		char clientmessage[32] = { 0 };
		char method[16] = { 0 };

		ev->len = len;
		ev->buf[len] = '\0';

		//printf("buf:%s\n", ev->buf);
		char* data = strstr(ev->buf, "\r\n\r\n");
		char* username = strstr(ev->buf, ":");
		strncpy(method, ev->buf, (size_t)(username - ev->buf));
		username += 1;
		strncpy(clientmessage, username, (size_t)(data - username));

		printf("method:%s\n", method);
		if (strcmp(method, "login") == 0)
		{
			strcpy(ev->alias, clientmessage);
			send(ev->fd,"true",4,0);
		}
		else if (strcmp(method, "request") == 0)
		{
			int i = 0;
			char msg[BUFSIZE];
			memset(msg, 0, BUFSIZE);
			for (; i < MAX_CONNECT_NUM; i++)
			{
				//存在连接
				if (g_events_info[i].status == 1 && strcmp(g_events_info[i].alias,clientmessage)!=0)
				{
					strcat(msg, "\r\n");
					strcat(msg, g_events_info[i].alias);
				}
				else
				{
					continue;
				}

			}
			strcat(msg, "\r\n\r\n");
			send(ev->fd, msg, BUFSIZE, 0);
			//

		}



		else if (strcmp(method, "send") == 0)
		{
			int i = 0;
			char msg[BUFSIZE];
			memset(msg, 0, BUFSIZE);
			for (;i<MAX_CONNECT_NUM; i++)
			{
				if (strcmp(g_events_info[i].alias, clientmessage) == 0)
					break;
			}
			data += 4;
			printf("need to send to fd:%d\t", g_events_info[i].fd);

			strcat(msg, "from:");
			strcat(msg, ev->alias);
			strcat(msg, "\r\n\r\n");
			strcat(msg, data);
			if (g_events_info[i].status == 1)
			{
				printf("packet:%s\n", msg);
				int fd = g_events_info[i].fd;
				send(fd, msg, strlen(data) + 1, 0);
			}
			else
			{
				const char* notfound = "notfound this user\n";
				send(ev->fd, notfound, strlen(notfound)+1, 0);
			}
		}


	}

	else if (len == 0)
	{
		printf("close :fd=%d\tpos=%d\n", fd, ev - g_events_info);
		close(ev->fd);
		del_event_info(g_efd, ev);
	}

	else
	{
		close(ev->fd);
		printf("recv [fd:%d] error:%s", fd, strerror(errno));
	}

	return;
}

int main(int argc, char* argv[])
{
	//创建epoll树的根节点
	g_efd = epoll_create(MAX_CONNECT_NUM + 1);
	if (g_efd < 0)
	{
		printf("create efd error\n");
		exit(1);
	}


	init_socket(g_efd);							//初始化

	struct epoll_event events[MAX_CONNECT_NUM + 1];



	while (1)
	{
		int i = 0;
		//监听efd，调用epoll_wait	阻塞1秒
		int fd_num = epoll_wait(g_efd, events, MAX_CONNECT_NUM + 1, 1000);
		if (fd_num < 0)
		{
			printf("epoll_wait error\n");
			exit(1);
		}

		for (i = 0; i < fd_num; i++)
		{
			eventInfo* evinfo = (eventInfo*)events[i].data.ptr;

			if ((events[i].events & EPOLLIN) && (evinfo->events & EPOLLIN))				//可读
			{
				evinfo->call_back(evinfo->fd, events[i].events, evinfo->arg);
			}
		}
	}

	return 0;
}