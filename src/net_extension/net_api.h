#pragma once

struct NetAPIRequest;
struct NetAPIResponse;

using NetInitAPIFn = void(*)();
using NetFreeAPIFn = void(*)();
using NetFormatAPIResponseFn = void*(*)(void* input, int32_t input_size, NetAPIRequest* request);
using NetHandleAPIResponseFn = void(*)(NetAPIResponse* response);
using NetFreeResponseFn = void(*)(NetAPIResponse* response);

struct NetAPIDesc
{
    // Called during Net_AllLoaded to create script natives and forwards.
    NetInitAPIFn init_func;

    // Called during Net_Shutdown to remove stuff, such as natives and forwards.
    NetFreeAPIFn free_func;

    // Allocate and return your response structure from the input bytes received.
    // Return NULL if the data is not what you expect.
    // Called in the net thread.
    NetFormatAPIResponseFn format_response_func;

    // Process the response, such as calling a script forward.
    // If the response worked, you can create a response handle through Net_MakeResponseHandle and pass that to the script.
    // You must not call Net_MakeResponseHandle or pass anything about the response to the script if the response failed.
    NetHandleAPIResponseFn handle_response_func;

    // Free anything previously allocated during the request and response.
    // This gets called directly after handle_response_func.
    NetFreeResponseFn free_response_func;
};

struct NetAPIRequest
{
    NetAPIDesc* desc; // Which API.
    wchar_t* request_string; // Formatted by the API. This is an addition to the host name, and should start with a slash.
    void* request_state; // Additional information to be retrived again during the response.
};

struct NetAPIResponse
{
    NetAPIDesc* desc; // Which API.
    bool status; // If we got anything at all.
    void* request_state; // Additional information set when the request was made.
    void* response_state; // Type specific response data.
};

bool Net_ConnectedToInet();
void Net_MakeHttpRequest(NetAPIDesc* desc, const wchar_t* request_string, void* request_state);

void Net_ClearHeaders();
void Net_AppendHeader(const wchar_t* str);
void Net_AddHeader(const wchar_t* format, ...);
void Net_TerminateHeader();

NetAPIResponse* Net_GetResponseFromHandle(int32_t response_handle, NetAPIDesc* type_check);
int32_t Net_MakeResponseHandle(NetAPIResponse* response);
