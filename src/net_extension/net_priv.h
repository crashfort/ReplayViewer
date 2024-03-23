#pragma once
#include "smsdk_ext.h"
#include <Windows.h>
#include <strsafe.h>

#define NET_LOCK(SRW) AcquireSRWLockExclusive((SRW))
#define NET_UNLOCK(SRW) ReleaseSRWLockExclusive((SRW))
#define NET_SPRINTFW(FORMAT, BUF, ...) StringCchPrintfW((BUF), ARRAYSIZE((BUF)), (FORMAT), __VA_ARGS__)

using NetAPIType = int;

enum /* NetAPIType */
{
    NET_API_GET_PLAYER_NAME
};

struct NetAPIRequest;
struct NetAPIResponse;

using NetInitAPIFn = void(*)();
using NetFreeAPIFn = void(*)();
using NetFormatAPIResponseFn = void(*)(void* input, int input_size, void* dest);
using NetHandleAPIResponseFn = void(*)(NetAPIResponse* response);

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

    // Format a response structure from the input, in net thread.
    // The dest parameter will be allocated according to response_size bytes, so you can cast that to your internal response structure.
    NetFormatAPIResponseFn format_response_func;

    // Process the response, such as calling a script event.
    NetHandleAPIResponseFn handle_response_func;
};

struct NetAPIRequest
{
    NetAPIDesc* desc; // Which API.
    wchar_t* request_string; // Formatted by the API.
};

struct NetAPIResponse
{
    NetAPIDesc* desc; // Which API.
    bool status; // If we got anything at all.
    void* data; // Type specific response data. This gets allocated and freed for you.
};

bool Net_ConnectedToInet();
void Net_MakeHttpRequest(NetAPIType type, const wchar_t* request_string);
