#pragma once
#include "smsdk_ext.h"
#include <Windows.h>
#include <strsafe.h>
#include <stdint.h>

#define NET_LOCK(SRW) AcquireSRWLockExclusive((SRW))
#define NET_UNLOCK(SRW) ReleaseSRWLockExclusive((SRW))

#define NET_SNPRINTFW(BUF, FORMAT, ...) StringCchPrintfW((BUF), ARRAYSIZE((BUF)), (FORMAT), __VA_ARGS__)
#define NET_VSNPRINTFW(BUF, FORMAT, VA) StringCchVPrintfW((BUF), ARRAYSIZE((BUF)), (FORMAT), VA)

using NetAPIType = int;

enum /* NetAPIType */
{
    NET_API_GET_PLAYER_NAME,
};

struct NetAPIRequest;
struct NetAPIResponse;

using NetInitAPIFn = void(*)();
using NetFreeAPIFn = void(*)();
using NetFormatAPIResponseFn = bool(*)(void* input, int input_size, void* dest, NetAPIRequest* request);
using NetHandleAPIResponseFn = void(*)(NetAPIResponse* response);
using NetAddHeadersFn = void(*)(NetAPIRequest* request);

struct NetAPIDesc
{
    // Identifier.
    NetAPIType type;

    // Size of response structure to be allocated.
    // The state will allocate this many bytes, so you can cast the data to your internal response structure.
    int response_size;

    // Called during init to create script events and other things.
    NetInitAPIFn init_func;

    // Called during free to remove stuff.
    NetFreeAPIFn free_func;

    // Format a response structure from the input.
    // The dest parameter will be allocated according to response_size bytes, so you can cast that to your internal response structure.
    // Called in the net thread.
    NetFormatAPIResponseFn format_response_func;

    // Process the response, such as calling a script event.
    NetHandleAPIResponseFn handle_response_func;

    // Optional function where you can add new or replace the headers in the http request.
    // Called in the net thread.
    NetAddHeadersFn set_headers_func;
};

struct NetAPIRequest
{
    NetAPIDesc* desc; // Which API.
    wchar_t* request_string; // Formatted by the API. This is an addition to the host name, and should start with a slash.
    void* request_state; // Additional information to be retrived again during the response. This gets freed for you.
};

struct NetAPIResponse
{
    NetAPIDesc* desc; // Which API.
    bool status; // If we got anything at all.
    void* data; // Type specific response data. This gets allocated and freed for you.
    void* request_state; // Additional information set when the request was made. This gets freed for you.
};

bool Net_ConnectedToInet();
void Net_MakeHttpRequest(NetAPIType type, const wchar_t* request_string, void* request_state);

void Net_ClearHeaders();
void Net_AppendHeader(const wchar_t* str);
void Net_AddHeader(const wchar_t* format, ...);
void Net_TerminateHeader();
