// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <WinSock2.h>
#include <process.h>
#include <math.h>		// ceil()
#include <string.h>		// memset()
#include <ctype.h>		// isprint()
#include <sys/stat.h>	// _stati64()


// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
#include <iostream>
#include <string>



typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long  uint64;

using namespace std;

#include "PacketAnalyzer.h"
#include "TSPacket.h"
#include "PESPacket.h"
#include "PATPacket.h"
#include "CATPacket.h"
#include "PMTPacket.h"