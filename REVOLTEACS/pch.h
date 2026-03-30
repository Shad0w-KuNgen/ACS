// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#include <Windows.h>
#include <d3d9.h>
#include <d3d11.h>
#include <dxgi.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <detours.h>
#include <vector>
#include <tchar.h>
#include <psapi.h>
#include <atlstr.h>
#include <fstream>
#include <shellapi.h>
#include <winnetwk.h>
#include <Iphlpapi.h>
#include <list>
#include "md5.h"
#include <utility>
#include "xorstr.h"
#include "crc32.h"
#include <wincrypt.h> 
#include <winternl.h>
#include "SkCrypter.h"
#include "Remap.h"
#include <set>
#include <map>
#include <unordered_map>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <iomanip> // std::setw

#include "types.h"
#include "ByteBuffer.h"
#include "Packet.h"
#include "PacketHandler.h"
#include "PlayerBase.h"
#include "UIManager.h"
#include "GameHooks.h"
#include "RenderSystem.h"
#include "UIFramework.h"
#include "UITaskbar.h"
#include "PearlEngine.h"
#define _CRT_SECURE_NO_WARNINGS
#define PERK_COUNT 13
#define UI_PERKCOUNT 13

#pragma comment(lib, "detours.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "detours.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mpr.lib")
//#pragma comment(lib, "VirtualizerSDK32.lib")

#endif //PCH_H
