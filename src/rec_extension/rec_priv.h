#pragma once
#include "smsdk_ext.h"
#include <Windows.h>
#include <WinInet.h>
#include <strsafe.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>

#include "rec_state.h"

#define REC_ARRAY_SIZE(A) (sizeof(A) / sizeof(A[0]))
#define REC_SNPRINTF(BUF, FORMAT, ...) StringCchPrintfA((BUF), REC_ARRAY_SIZE((BUF)), (FORMAT), __VA_ARGS__)
#define REC_SNPRINTFW(BUF, FORMAT, ...) StringCchPrintfW((BUF), REC_ARRAY_SIZE((BUF)), (FORMAT), __VA_ARGS__)
#define REC_VSNPRINTFW(BUF, FORMAT, VA) StringCchVPrintfW((BUF), REC_ARRAY_SIZE((BUF)), (FORMAT), VA)
#define REC_COPY_STRING(SOURCE, DEST) StringCchCopyA((DEST), REC_ARRAY_SIZE((DEST)), (SOURCE))
#define REC_COPY_STRINGW(SOURCE, DEST) StringCchCopyW((DEST), REC_ARRAY_SIZE((DEST)), (SOURCE))
