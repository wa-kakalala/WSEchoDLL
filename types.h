#pragma once

typedef struct Message {
	unsigned char type;
	unsigned int  len;
	char data[512];
}Message;

typedef enum MesType {
	MES_PING_REQ,
	MES_PING_REP
}MesType;

