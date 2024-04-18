#pragma once
#include "smsdk_ext.h"
#include <Windows.h>
#include <WinInet.h>
#include <strsafe.h>
#include <stdint.h>
#include <stdarg.h>
#include <vector>
#include <assert.h>
#include "json.h"

#include "net_state.h"
#include "net_util.h"
#include "net_api.h"
#include "net_apis.h"

#define NET_LOCK(SRW) AcquireSRWLockExclusive((SRW))
#define NET_UNLOCK(SRW) ReleaseSRWLockExclusive((SRW))

#define NET_ARRAY_SIZE(A) (sizeof(A) / sizeof(A[0]))
#define NET_SNPRINTFW(BUF, FORMAT, ...) StringCchPrintfW((BUF), NET_ARRAY_SIZE((BUF)), (FORMAT), __VA_ARGS__)
#define NET_VSNPRINTFW(BUF, FORMAT, VA) StringCchVPrintfW((BUF), NET_ARRAY_SIZE((BUF)), (FORMAT), VA)
#define NET_COPY_STRING(SOURCE, DEST) StringCchCopyA((DEST), NET_ARRAY_SIZE((DEST)), (SOURCE))
