#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "types.h"
#include "WSEcho.h"
#include <winsock2.h>
// #include <time.h>
#pragma comment(lib,"ws2_32.lib")

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
		perror("socket-lib init error");
		return -1;
	}
	//byte lv = LOBYTE(wsaData.wVersion);
	//byte hv = HIBYTE(wsaData.wHighVersion);
	//printf("current socket lib verison: \r\n");
	//printf("%d.%d\r\n", hv, lv);
	//printf("\r\n");
	return 0;
}

/**
* socket 绑定
* @param	ip			ipv4地址
* @param	port		端口
* @return	socketfd	监听的socketfd
*           -1          fail
*/
int Bind(const char*ip, unsigned short* port) {
	if(WSA_Init()<0) return -1;
	sfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sfd == INVALID_SOCKET) {
		perror("invaild socket");
		return -1;
	}
	struct sockaddr_in srv_addr;
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(*port);
	srv_addr.sin_addr.s_addr = inet_addr(ip);
	if (srv_addr.sin_addr.s_addr == INADDR_NONE)
	{
		perror("address\r\n");
		return -1;
	}
	if ( bind(sfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr) ) == SOCKET_ERROR ) {
		perror("bind error");
		return -1;
	}
	return sfd;
}

int Close() {
	if(closesocket(sfd) == SOCKET_ERROR) return -1;
	WSACleanup();
	return 0;
}

int Receive(void(*f)(const char * msg)) {
	char buf[1024] = { 0 };
	struct sockaddr_in clnt_addr; /* 客户端地址 */
	int res = 0;
	int addr_len = sizeof(clnt_addr);
	res = recvfrom(sfd, buf, 512, 0,
		(struct sockaddr *)&clnt_addr, &addr_len);
	Message * msg = (Message*)buf;
	switch (msg->type) {
	case MES_PING_REQ:
		f(msg->data);
		memset(msg, 0, sizeof(buf));
		msg->type = MES_PING_REP;
		sprintf(msg->data, "%s", "hello,wkk to client");
		msg->len = strlen(msg->data);
		sendto(sfd, (char *)msg, sizeof(unsigned int) + 1 + msg->len, 0,
			(struct sockaddr *)&clnt_addr, addr_len);
		break;
	}
	return 0;
}

void StopReceive() {

}

int Ping(const char* ip, unsigned short port) {
	if (WSA_Init() < 0) return -1;
	SOCKET  clientfd = 0;
	int res = 0;
	struct sockaddr_in peer_addr, serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	if (serv_addr.sin_addr.s_addr == INADDR_NONE)
	{
		perror("address\r\n");
		return -1;
	}

	clientfd = socket(AF_INET, SOCK_DGRAM, 0);

	Message pingMsg = { MES_PING_REQ };

	/*time_t now = time(&now);
	struct tm* nowTime = localtime(&now);
	if (nowTime == NULL) {
		perror("parse localtime error");
		return -1;
	}*/
	/*sprintf(pingMsg.data,"%d/%d/%d %d:%d:%d\0",nowTime->tm_year,nowTime->tm_mon,nowTime->tm_mday
										   ,nowTime->tm_hour,nowTime->tm_min,nowTime->tm_sec);*/
	sprintf(pingMsg.data, "%s", "hello,wkk to server");
	pingMsg.len = strlen(pingMsg.data);

	res = sendto(clientfd, (char*)&pingMsg, (sizeof(unsigned int) + 1 + pingMsg.len), 0,
		(struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (res == SOCKET_ERROR) {
		perror("send data error");
		return -1;
	}
	memset(&pingMsg, 0, sizeof(pingMsg));
	res = recvfrom(clientfd, (char*)&pingMsg, 512, 0, NULL, 0);
	if (res == SOCKET_ERROR) {
		perror("recv data error");
		return -1;
	}
	printf("%s\r\n", pingMsg.data);
	closesocket(clientfd);
	WSACleanup();
	
	return 0;
}
