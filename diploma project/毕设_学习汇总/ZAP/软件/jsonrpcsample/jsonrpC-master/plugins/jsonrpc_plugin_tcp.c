/*
 * Copyright (c) 2012 Jonghyeok Lee <jhlee4bb@gmail.com>
 *
 * jsonrpC is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "jsonrpc_plugin_tcp.h"

#ifndef WIN32
#include<lwip/sockets.h>
//#include<netinet/in.h>
//#include <arpa/inet.h>
#else
#include <math.h>
#include <stdarg.h>
#include<string.h>


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <memory.h>

#include <winsock2.h>
#include <unistd.h>
#endif

#include "Logger.h"
#include "Ringbuf16.h"
DEFINE_LOGGER(tcp, 128, LOG_DEBUG);


#ifdef WIN32
LPSTR getstrerror(void)
{
    HLOCAL LocalAddress=NULL;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,GetLastError(),0,(PTSTR)&LocalAddress,0,NULL);
    return (LPSTR)LocalAddress;
}
#else
// const char *getstrerror(void)
// {
// 	return strerror(errno);
// }
#endif




typedef struct {
	int server_socket;
	int client_socket;
	int serverPort;
	int error;
	struct sockaddr_in remoteAddr;

	Ringbuf16_ST rcvRingbuf;
	Ringbuf16_ST sndRingbuf;

	unsigned char recv_buf[2048];
	unsigned char send_buf[2048];

	unsigned char recvMsg[2048];
	int rcvedOffset;

	int parseState; //0-idle, 1-recved 0xdc, 2-recved 0xcd 3-recved 0x01 4-recved optionbyte
	unsigned char recvOptions;
} Jsonrpc_Tcp_ST;


#define PARSE_STATE_IDLE				0
#define PARSE_STATE_RCVED_MAGIC1		1
#define PARSE_STATE_RCVED_MAGIC2		2
#define PARSE_STATE_RCVED_VERSION		3
#define PARSE_STATE_RCVED_OPTIONS		4
#define PARSE_STATE_WAIT_PROCESS		5


Jsonrpc_Tcp_ST *	Jsonrpc_TcpServerInit (int serverPort)
{
	//*****socket init******//
	//*****return : p->server_socket*****//
	struct sockaddr_in servaddr;

	Jsonrpc_Tcp_ST * p = malloc(sizeof(Jsonrpc_Tcp_ST));

	if(p == NULL)
	{
		return NULL;
	}

	p->error = 0;
	p->server_socket = -1;
	p->client_socket = -1;
	p->serverPort = serverPort;

	p->parseState = 0;

	Ringbuf16_Init(&p->rcvRingbuf, p->recv_buf, sizeof(p->recv_buf));
	Ringbuf16_Init(&p->sndRingbuf, p->send_buf, sizeof(p->send_buf));

	printf("p->serverPort = %d\n",p->serverPort);

#ifdef WIN32
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		printf("Init Windows Socket Failed::0x%x\n", GetLastError());
		return NULL;
	}
#endif

	p->server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (p->server_socket == -1) {
		printf("socket create error 0x%x\n",errno);
		free(p);
		return NULL;
	}

	printf("create socket success\n");


	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(p->serverPort);
	if (bind(p->server_socket, (struct sockaddr*) &servaddr, sizeof(servaddr))== -1)
	{
		close(p->server_socket);
		free(p);
		return NULL;
	}
	if (listen(p->server_socket, 2) == -1) {
		close(p->server_socket);
		free(p);
		return NULL;
	}

	printf("waiting for client connect\n");

	return p;
}





Jsonrpc_Tcp_ST *	Jsonrpc_TcpClientInit (const char * serverAddr, int serverPort)
{
	//*****socket init******//
	//*****return : p->server_socket*****//
	struct sockaddr_in servaddr;

	Jsonrpc_Tcp_ST * p = malloc(sizeof(Jsonrpc_Tcp_ST));

	if(p == NULL)
	{
		return NULL;
	}

	p->error = 0;
	p->server_socket = -1;
	p->client_socket = -1;
	p->serverPort = serverPort;

	p->parseState = 0;

	Ringbuf16_Init(&p->rcvRingbuf, p->recv_buf, sizeof(p->recv_buf));
	Ringbuf16_Init(&p->sndRingbuf, p->send_buf, sizeof(p->send_buf));

	printf("p->serverPort = %d\n",p->serverPort);

#ifdef WIN32
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		printf("Init Windows Socket Failed::0x%x\n", GetLastError());
		return NULL;
	}
#endif

	p->client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (p->client_socket == -1) {
		printf("socket create error 0x%x\n",errno);
		free(p);
		return NULL;
	}

	printf("create socket success\n");


	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(serverAddr);
	servaddr.sin_port = htons(p->serverPort);

	if (connect(p->client_socket, (const struct sockaddr*) &servaddr, sizeof(servaddr))== -1)
	{
		close(p->client_socket);
		free(p);
		return NULL;
	}


	return p;
}
static int DoSelect(Jsonrpc_Tcp_ST * self)
{
	//*****select init*******//
	int sock_len = sizeof(struct sockaddr);
	int client_socket = -1;
	int result = 0;
	int nfds;
	struct fd_set readfds;
	struct fd_set writefds;
	struct fd_set exceptfds;
	struct timeval timeout={0,0}; //select等待3秒，3秒轮询，要非阻塞就置0

	FD_ZERO(&readfds);			//****用之前先将其清零****//
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);			//****用之前先将其清零****//

	if(self->server_socket != -1)		//****socket是否成功产生***//
	{
		FD_SET(self->server_socket,&readfds); //是，则添加描述符
		FD_SET(self->server_socket,&exceptfds); //添加描述符
	}
	if(self->client_socket != -1)			//****看是否有客户端连接进来***//
	{
		FD_SET(self->client_socket,&readfds); //如果有，添加描述符
		if(Ringbuf16_Elements(&self->sndRingbuf))/*sndRingbuf非空*/
		{
			FD_SET(self->client_socket,&writefds);
		}
		FD_SET(self->client_socket,&exceptfds); //添加描述符
	}

	nfds = (self->server_socket > self->client_socket) ? self->server_socket + 1 : self->client_socket + 1;


	if(nfds > 0)
	{
	//***循环检测描述符合集的变化******//
	result = select(nfds, &readfds, &writefds, &exceptfds, &timeout);			//****监听检查结果***//
	if( result < 0 )
	{
		return -1;
	}
	else if( result == 0 )
	{
		return 0;
	}

	if(FD_ISSET(self->server_socket,&readfds))		//****服务器端已经成功打开socket，之后的连接与断开不再影响该值的变化***//
	{
		printf("server_socket  readable*********\n");			//****socket成功打开信息显示***//
		{
			client_socket = accept(self->server_socket, (struct sockaddr *)&self->remoteAddr, &sock_len);		//***进行接收***//
			if(client_socket > 0)
			{
				if(self->client_socket > 0)
				{
					/*只支持1个client，如果原来有client，则不允许新的client*/
					printf("client_socket has connected already, close new client\n");
					close(client_socket);
				}
				else
				{
					self->client_socket = client_socket;
					printf("client %s connected\n", inet_ntoa(self->remoteAddr.sin_addr));			//****接受到了新的客户端的连接**//
				}

			}
			else
			{
				printf("accept error %d\n", 111);
			}
		}
	}
	if(FD_ISSET(self->server_socket,&exceptfds))		//******服务器打开socket异常***//
	{
		printf("server_socket exception\n");			//*****//
	}

	if(FD_ISSET(self->client_socket,&readfds))				//**连接字描述符处于可读状态，表示处于可以连接客户端的状态******//
	{
		printf("client_socket readable**********\n");		//可以空值返回，让其处于一直轮询监听状态//


		unsigned char * ptr = Ringbuf16_GetPutPtr(&self->rcvRingbuf);		//******数据写入的入口地址***//
		int maxlen = Ringbuf16_GetPutLength(&self->rcvRingbuf, TRUE);				//***允许写入的最大长度****//

		printf("ptr=%x maxlen=%d put=%d get=%d mask=%d\n", (unsigned int)ptr, maxlen, self->rcvRingbuf.put_ptr, self->rcvRingbuf.get_ptr, self->rcvRingbuf.mask);

		int recv_data = recv(self->client_socket, (char *)ptr, maxlen, 0);			//*****接收数据，并将其根据得到的ringbuf写数据地址指针，写入ringbuf****//
		if(maxlen <= 0)
		{
		    printf("buffer insuffcient\n");
		}
		else
		{	//***数据接收结果；数据写入地址；最大长度；数据写；数据读；******//
		if(recv_data <= 0)/*如果CLIENT连接断开，收到长度为为0*/
		{
			printf("client_socket disconnected\n");
			close(self->client_socket);
			self->client_socket = -1;
		}
		else
		{

			LogHexDump(LOG_DEBUG,"recv from client", ptr, recv_data);		//**打印接收到的数据（16进制输出）**//
			Ringbuf16_PutPtrInc(&self->rcvRingbuf, recv_data);			//**接收地址指针递增**//
		}
		}
	}
	//**SendRingBuf在外部接口函数中实现数据的多方面调用来完成写入，在此处将其取出，并发送**//
	if(FD_ISSET(self->client_socket,&writefds))			//****判断是否可写****//
	{

		printf("client_socket writeable\n");
		//**取出指定长度的数据，将其发送**//
		unsigned char * ptr = Ringbuf16_GetGetPtr(&self->sndRingbuf);			//**得到读指针**//
		int remain = Ringbuf16_GetGetLength(&self->sndRingbuf, TRUE);			//**得到发送数据的长度**//
		printf("ptr=%x remain=%d put=%d get=%d mask=%d\n", (unsigned int)ptr, remain, self->sndRingbuf.put_ptr, self->sndRingbuf.get_ptr, self->sndRingbuf.mask);
		int send_data = send(self->client_socket, (char *)ptr, remain, 0);		//**发送的时候要指定长度，因此必须在之前先获得可发送数据的长度**//
		if(send_data < 0)
		{
			printf("send failed\n");
		}
		else {
			if(remain != send_data)		//**没有发送指定的长度的数据**//

		{
			printf("%d bytes sent, %d bytes expected\n", send_data, remain);
		}
		else
		{
			printf("%d bytes sent\n", send_data);
		}
			Ringbuf16_GetPtrInc(&self->sndRingbuf, send_data);		//**读指针同样需要递增**//
		}




	}
	if(FD_ISSET(self->client_socket,&exceptfds))			//****客户端连接异常***//
	{
		printf("client_socket exception\n");
	}
	return 1;


	}

	else
	{
		return 0;

	}
}

//**依据self中的解析位的具体状态和接收到的数据，来分析**//
static void Jsonrpc_TcpParse(Jsonrpc_Tcp_ST * self, unsigned char rcvChar)
{
	switch(self->parseState)
	{
	case PARSE_STATE_IDLE:
		if(rcvChar == 0xDC)
		{
			self->parseState = PARSE_STATE_RCVED_MAGIC1;
		}
		break;
	case PARSE_STATE_RCVED_MAGIC1:
		if(rcvChar == 0xCD)
		{
			self->parseState = PARSE_STATE_RCVED_MAGIC2;
		}
		else
		{
			self->parseState = PARSE_STATE_IDLE;
		}
		break;
	case PARSE_STATE_RCVED_MAGIC2:
		if(rcvChar == 0x01)
		{
			self->parseState = PARSE_STATE_RCVED_VERSION;
		}
		else
		{
			self->parseState = PARSE_STATE_IDLE;
		}
		break;
	case PARSE_STATE_RCVED_VERSION:
		if((rcvChar == 0x03)||(rcvChar == 0x07))
		{
			self->parseState = PARSE_STATE_RCVED_OPTIONS;
			self->recvOptions = rcvChar;
			self->rcvedOffset = 0;
		}
		else
		{
			self->parseState = PARSE_STATE_IDLE;
		}
		break;
	case PARSE_STATE_RCVED_OPTIONS:
		self->recvMsg[self->rcvedOffset] = rcvChar;
		if(self->rcvedOffset < sizeof(self->recvMsg)-1)
		{
			self->rcvedOffset++;
		}
		if(rcvChar == '\0')
		{
			self->parseState = PARSE_STATE_WAIT_PROCESS;
		}
		break;
	case PARSE_STATE_WAIT_PROCESS:
//		if(self->rcvedOffset == 0)
//		{
//			self->parseState = PARSE_STATE_IDLE;
//		}
		break;
	default:
		self->parseState = PARSE_STATE_IDLE;
		break;
	}
}
//**函数功能:接收连接，接收数据，发送数据，并按位解析数据**//
static jsonrpc_error_t Jsonrpc_TcpTick(jsonrpc_handle_t net)
{
	Jsonrpc_Tcp_ST * self = (Jsonrpc_Tcp_ST *)net;

	DoSelect(self);			//**调用select,完成客户端的接收与数据请求的写入，select在完成一次调用后会跳出函数**//
	if(self->parseState != PARSE_STATE_WAIT_PROCESS)
	{
	int rcvChar = Ringbuf16_Get(&self->rcvRingbuf);			//**取出接收到的值**//
	if(rcvChar != -1)
	{
		Jsonrpc_TcpParse(self, rcvChar);		//**解析接收到的值**//
	}
	}
	return JSONRPC_ERROR_OK;
}


static void	Jsonrpc_TcpClose(jsonrpc_handle_t net)
{
	Jsonrpc_Tcp_ST * self = (Jsonrpc_Tcp_ST *)net;

	if(-1 != self->server_socket)
	{
	close(self->server_socket);
	}
	if(-1 != self->client_socket)
	{
		close(self->client_socket);
	}

	free(self);
}
//**根据传入的参数，完成客户端的，或是服务器端的socket打开**//
static jsonrpc_handle_t	Jsonrpc_TcpOpen (va_list ap)
{
	int client;
	int serverPort;
	const char * serverAddr;
	Jsonrpc_Tcp_ST * ret;

	client = va_arg(ap, int);
	serverPort = va_arg(ap, int);
	if(client != 0)
	{
		serverAddr = va_arg(ap, const char *);
		ret = Jsonrpc_TcpClientInit(serverAddr, serverPort);
	}
	else
	{
		ret = Jsonrpc_TcpServerInit(serverPort);
	}
    return (jsonrpc_handle_t)ret;
}

//**对调用结果进行固定格式的包装**//
static jsonrpc_error_t Jsonrpc_TcpSend(jsonrpc_handle_t net, const char *data, void *desc)
{
	int ret = 0;
	int strlength;
	Jsonrpc_Tcp_ST * self = (Jsonrpc_Tcp_ST *)net;
	//**对调用结果进行指定格式的包装**//
	ret += Ringbuf16_Put(&self->sndRingbuf, 0xDC);
	ret += Ringbuf16_Put(&self->sndRingbuf, 0xCD);
	ret += Ringbuf16_Put(&self->sndRingbuf, 0x00);
	ret += Ringbuf16_Put(&self->sndRingbuf, 0x03);
	strlength = strnlen(data, 2040);			//**计算data的长度**//
	ret += Ringbuf16_PutArray(&self->sndRingbuf, data, strlength);		//**写入调用结果**//
	ret += Ringbuf16_Put(&self->sndRingbuf, 0x00);
	if(ret == strlength + 5)		//**查询是否全部写入**//
	{
		return JSONRPC_ERROR_OK;
	}
	else
	{
		return JSONRPC_ERROR_SERVER_OUT_OF_MEMORY;
	}
}

//**对接收到的数据，进行解析状态标志位的初始化**//
static const char * Jsonrpc_TcpRecv(jsonrpc_handle_t net, unsigned int timeout, void **desc)
{
	Jsonrpc_Tcp_ST * self = (Jsonrpc_Tcp_ST *)net;
	if(self->parseState != PARSE_STATE_WAIT_PROCESS)
	{
		return NULL;
	}
	else
	{
		self->parseState = PARSE_STATE_IDLE;
		return (const char *)self->recvMsg;
	}
}


static jsonrpc_error_t Jsonrpc_TcpError (jsonrpc_handle_t net)
{
	Jsonrpc_Tcp_ST * self = (Jsonrpc_Tcp_ST *)net;

	return self->error;
}



const jsonrpc_net_plugin_t	* jsonrpc_plugin_tcp (void)
{
	static const jsonrpc_net_plugin_t plugin_tcp = {
			Jsonrpc_TcpOpen,
			Jsonrpc_TcpClose,
		Jsonrpc_TcpRecv,
		Jsonrpc_TcpSend,
		Jsonrpc_TcpTick
	};
	return &plugin_tcp;
}

