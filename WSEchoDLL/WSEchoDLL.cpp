#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "types.h"
#include "WSEchoDLL.h"
#include <winsock2.h>
#include <time.h>
#pragma comment(lib,"ws2_32.lib")

/*
	0:  启动
	1： 暂停
	2： 退出
*/

SOCKET sfd = 0;

/**
* 初始化WSA
* 操作系统根据请求的Socket版本搜索相应的Socket库 --> 绑定到该应用程序
* @return -1 fail
		   0 success
*/
int WSA_Init() {
	WORD sockVersion = MAKEWORD(2, 2); // socket verison 
	WSADATA wsaData = { 0 };
	if (WSAStartup(sockVersion, &wsaData) != 0) {
		//perror("socket-lib init error");
		return -1;
	}
	//byte lv = LOBYTE(wsaData.wVersion);
	//byte hv = HIBYTE(wsaData.wHighVersion);
	//printf("current socket lib verison: \r\n");
	//printf("%d.%d\r\n", hv, lv);
	//printf("\r\n");
	return 0;
}

int Init() {
	if(WSA_Init()<0) return -1;
	return 0;
}

void Defer() {
	WSACleanup();
}

/**
* socket 绑定
* @param	ip			ipv4地址
* @param	port		端口
* @return	socketfd	监听的socketfd
*           -1          fail
*/
int Bind(const char*ip, unsigned short* port) {
	int res = 0;
	sfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sfd == INVALID_SOCKET) {
		perror("invaild socket");
		return -1;
	}
	struct sockaddr_in srv_addr;
	int addr_len = sizeof(srv_addr);
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(*port);
	srv_addr.sin_addr.s_addr = inet_addr(ip);
	if (srv_addr.sin_addr.s_addr == INADDR_NONE)
	{
		perror("address\r\n");
		return -1;
	}
	if ( bind(sfd, (struct sockaddr *)&srv_addr, addr_len) == SOCKET_ERROR ) {
		perror("bind error");
		return -1;
	}

	int iTimeOut = 1;
	setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeOut, sizeof(iTimeOut));
	// 获取端口信息
	res = getsockname(sfd, (struct sockaddr *)&srv_addr, &addr_len); 
	*port = ntohs(srv_addr.sin_port);
	return sfd;
}

int Close() {
	if(closesocket(sfd) == SOCKET_ERROR) return -1;
	return 0;
}

int Receive(void(*f)(const char * msg)) {
	Message msg = { 0 };
	struct sockaddr_in clnt_addr; /* 客户端地址 */
	int res = 0;
	int addr_len = sizeof(clnt_addr);
	time_t now;
	struct tm *timeNow;

	res = recvfrom(sfd, (char*)&msg, sizeof(msg), 0, (struct sockaddr *)&clnt_addr, &addr_len);
	if (res < 0) return -1;

	switch (msg.type) {
	case MES_PING_REQ:
		f(msg.data);
		memset(&msg, 0, sizeof(msg));
		msg.type = MES_PING_REP;
		time(&now);
		timeNow = localtime(&now);
		sprintf(msg.data, "%d-%d-%d %d:%d:%d\r\n", timeNow->tm_year + 1900, timeNow->tm_mon + 1, timeNow->tm_mday,
			timeNow->tm_hour, timeNow->tm_min, timeNow->tm_sec);
		msg.len = strlen(msg.data);
		sendto(sfd, (char *)&msg, sizeof(unsigned int) + 5 + msg.len, 0,
			(struct sockaddr *)&clnt_addr, addr_len);
		break;
	}
}

void StopReceive() {

}


int Ping(const char* ip, unsigned short port) {
	SOCKET  clientfd = 0;
	int res = 0;
	Message pingMsg = { 0 };
	struct sockaddr_in serv_addr,client_addr;
	int addr_len = sizeof(serv_addr);
	time_t now;
	struct tm *timeNow;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	if (serv_addr.sin_addr.s_addr == INADDR_NONE)
	{
		perror("address\r\n");
		return -1;
	}

	clientfd = socket(AF_INET, SOCK_DGRAM, 0);

	pingMsg.type = MES_PING_REQ;
	time(&now);
	timeNow = localtime(&now);
	sprintf(pingMsg.data, "[client]-> %d-%d-%d %d:%d:%d\r\n",timeNow->tm_year+1900,
		      timeNow->tm_mon+1,timeNow->tm_mday,timeNow->tm_hour,timeNow->tm_min,timeNow->tm_sec);
	pingMsg.len = strlen(pingMsg.data);

	res = sendto(clientfd, (char*)&pingMsg, sizeof(unsigned int) + 5 + pingMsg.len, 0,
		(struct sockaddr *)&serv_addr, addr_len);
	if (res == SOCKET_ERROR) {
		perror("send data error");
		return -1;
	}
	memset(&pingMsg, 0, sizeof(pingMsg));
	res = recvfrom(clientfd, (char*)&pingMsg, sizeof(pingMsg), 0, (struct sockaddr *)&serv_addr, &addr_len);
	if (res == SOCKET_ERROR) {
		perror("recv data error");
		return -1;
	}
	//printf("%s\r\n", pingMsg.data);
	closesocket(clientfd);
	return 0;
}
