// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
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


// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
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