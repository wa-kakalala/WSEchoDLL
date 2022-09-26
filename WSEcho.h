#pragma once

#ifdef _DLLAPI
#define DLLAPI __declspec(dllexport)  // ����
#else
#define DLLAPI __declspec(dllimport)  // ����
#endif

#include <stdio.h>

extern "C" DLLAPI int Bind(const char*ip, unsigned short* port);
extern "C" DLLAPI int Close();
extern "C" DLLAPI int Receive(void(*f)(const char*msg));
extern "C" DLLAPI void StopReceive();
extern "C" DLLAPI int Ping(const char*ip, unsigned short port);