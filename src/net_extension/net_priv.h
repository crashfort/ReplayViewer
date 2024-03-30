#pragma once
#include "smsdk_ext.h"
#include <Windows.h>
#include <WinInet.h>
#include <strsafe.h>
#include <stdint.h>
#include <stdarg.h>
#include <vector>
#include "json.h"

#include "net_state.h"
#include "net_util.h"
#include "net_api.h"
#include "net_apis.h"

#define NET_LOCK(SRW) AcquireSRWLockExclusive((SRW))
#define NET_UNLOCK(SRW) ReleaseSRWLockExclusive((SRW))

#define NET_SNPRINTFW(BUF, FORMAT, ...) StringCchPrintfW((BUF), ARRAYSIZE((BUF)), (FORMAT), __VA_ARGS__)
#define NET_VSNPRINTFW(BUF, FORMAT, VA) StringCchVPrintfW((BUF), ARRAYSIZE((BUF)), (FORMAT), VA)
