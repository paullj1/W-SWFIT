#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "targetver.h"

#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>
#include <tchar.h>

#include "Library.h"
#include "Function.h"
#include "Operator.h"
#include "globals.h"
