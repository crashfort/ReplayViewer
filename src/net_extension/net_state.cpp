#include "net_priv.h"

// Quick and dirty code to retrieve server information for the replay viewer.
// If this would ever grow to do more stuff then put in proper structure.

const wchar_t* NET_HOST = L"74.91.123.152";
const int32_t NET_PORT = 3000;

// Headers to send with a request.
// Only used by the net thread.
wchar_t net_headers[8192];
wchar_t* net_headers_ptr; // Offset from start of net_headers where to append more headers.
size_t net_headers_rem; // Space remaining in net_headers for more headers.

// For API access.
wchar_t net_auth_token[128];

HINTERNET net_inet_h;
HINTERNET net_session_h; // Host session can remain open throughout.

// Event set by the main thread to notify that there are new requests to make.
HANDLE net_wake_event_h;

// Thread used for blocking network calls.
HANDLE net_thread_h;

SRWLOCK net_request_lock; // Protection for net_requests.
SRWLOCK net_response_lock; // Protection for net_responses.

// Bytes read from the request.
// Only used in the net thread.
std::vector<uint8_t> net_response_buffer;

// Requests we are sending to the net thread.
// Written by the main thread, read by the net thread.
std::vector<NetAPIRequest> net_requests;

// Copy of net_requests which are processed outside the lock.
// Only used by the net thread.
std::vector<NetAPIRequest> net_local_requests;

// Responses from the net thread.
// Written by the net thread, read by the main thread.
std::vector<NetAPIResponse> net_responses;

// Copy of net_responses which are given to scripts outside the lock.
// Only used in the main thread.
std::vector<NetAPIResponse> net_local_responses;

// Current response from the server.
// Used to allow reading the response headers.
// Only used by the net thread.
HINTERNET net_cur_response;

// Entrypoint for API calls.
// Call on the main thread to give a new http request to the net thread.
void Net_MakeHttpRequest(NetAPIDesc* desc, const wchar_t* request_string, void* request_state)
{
    NetAPIRequest req;
    req.desc = desc;
    req.request_string = wcsdup(request_string);
    req.request_state = request_state;

    NET_LOCK(&net_request_lock);
    net_requests.push_back(req);
    NET_UNLOCK(&net_request_lock);

    SetEvent(net_wake_event_h); // Notify net thread.
}

DWORD Net_QueryHttpNumber(HINTERNET req_h, DWORD query_for)
{
    DWORD ret = 0;
    DWORD size = sizeof(DWORD);
    HttpQueryInfoW(req_h, query_for | HTTP_QUERY_FLAG_NUMBER, &ret, &size, NULL);

    return ret;
}

bool Net_QueryHttpHeader(HINTERNET req_h, const wchar_t* header, wchar_t* dest, int32_t dest_size)
{
    // Querying for HTTP_QUERY_CUSTOM requires the input and output to be placed in the same buffer.

    wchar_t buf[1024];
    NET_COPY_STRINGW(header, buf);

    DWORD size = sizeof(buf); // Must be size in bytes, not chars.

    if (!HttpQueryInfoW(req_h, HTTP_QUERY_CUSTOM, buf, &size, NULL))
    {
        return false;
    }

    StringCchCopyW(dest, dest_size, buf);
    return true;
}

int32_t Net_ReadResponse(HINTERNET response_h, void* buf, int32_t buf_size)
{
    DWORD actually_read = 0;
    InternetReadFile(response_h, buf, buf_size, &actually_read);

    return actually_read;
}

DWORD NET_REQ_FLAGS = INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_KEEP_CONNECTION;

// Send request to server.
// If the return value is valid, you can call Net_ReadHttpResponse.
HINTERNET Net_SendHttpRequest(NetAPIRequest* request)
{
    // Note that you cannot use HTTP/2 because WinInet does not support it even when the web server is properly configured!
    HINTERNET req_h = HttpOpenRequestW(net_session_h, L"GET", request->request_string, L"HTTP/1.1", NULL, NULL, NET_REQ_FLAGS, 0);

    if (req_h == NULL)
    {
        // Cannot print here so need to make another mechanism.
        DWORD error = GetLastError();

        goto rfail;
    }

    if (req_h)
    {
        Net_ClearHeaders();
        Net_AddHeader(L"KSF_AUTH_TOKEN: Bearer %s", net_auth_token);

        if (request->desc->add_headers_func)
        {
            request->desc->add_headers_func(request);
        }

        Net_TerminateHeader();

        if (!HttpSendRequestW(req_h, net_headers, -1, NULL, 0))
        {
            // Cannot print here so need to make another mechanism.
            DWORD error = GetLastError();

            goto rfail;
        }
    }

    goto rexit;

rfail:
    if (req_h)
    {
        InternetCloseHandle(req_h);
        req_h = NULL;
    }

rexit:
    return req_h;
}

// Read bytes of response into net_response_buffer.
bool Net_ReadHttpResponse(HINTERNET req_h)
{
    net_response_buffer.clear();

    bool ret = false;

    DWORD code = Net_QueryHttpNumber(req_h, HTTP_QUERY_STATUS_CODE);

    if (code == HTTP_STATUS_OK)
    {
        uint8_t buf[2048];

        while (true)
        {
            int32_t num_read = Net_ReadResponse(req_h, buf, sizeof(buf));

            if (num_read == 0)
            {
                break;
            }

            net_response_buffer.insert(net_response_buffer.end(), buf, buf + num_read);
        }

        if (net_response_buffer.size() > 0)
        {
            ret = true;
        }
    }

    return ret;
}

// Give a response back to the main thread.
void Net_ReturnHttpResponse(NetAPIResponse* response)
{
    NET_LOCK(&net_response_lock);
    net_responses.push_back(*response);
    NET_UNLOCK(&net_response_lock);
}

DWORD CALLBACK Net_ThreadProc(LPVOID param)
{
    bool run = true;

    while (run)
    {
        WaitForSingleObject(net_wake_event_h, INFINITE);

        NET_LOCK(&net_request_lock);
        net_local_requests = std::move(net_requests);
        NET_UNLOCK(&net_request_lock);

        // Will be set to 0 on exit.
        run = net_local_requests.size() > 0;

        if (!run)
        {
            break;
        }

        for (size_t i = 0; i < net_local_requests.size(); i++)
        {
            NetAPIRequest* request = &net_local_requests[i];
            HINTERNET req_h = Net_SendHttpRequest(request);

            NetAPIResponse response = {};
            response.desc = request->desc;
            response.status = false;
            response.request_state = request->request_state;

            if (req_h)
            {
                response.status = Net_ReadHttpResponse(req_h);

                if (response.status)
                {
                    net_cur_response = req_h;

                    // The API must format the response now.
                    response.response_state = request->desc->format_response_func(net_response_buffer.data(), net_response_buffer.size(), request);
                    response.status = response.response_state != NULL;

                    net_cur_response = NULL;
                }

                InternetCloseHandle(req_h);
                req_h = NULL;
            }

            // Return the response. It may or may not have worked.
            Net_ReturnHttpResponse(&response);

            Net_Free(request->request_string);
        }

        net_local_requests.clear();
    }

    return 0; // Not used.
}

void Net_ClearHeaders()
{
    net_headers[0] = 0;

    net_headers_ptr = net_headers;
    net_headers_rem = NET_ARRAY_SIZE(net_headers);
}

// Add a fixed string header.
// Call Net_TerminateHeader after you have added your headers.
void Net_AppendHeader(const wchar_t* str)
{
    StringCchCatExW(net_headers_ptr, net_headers_rem, str, &net_headers_ptr, &net_headers_rem, 0);
    Net_TerminateHeader();
}

// Add a formatted string header.
// Call Net_TerminateHeader after you have added your headers.
void Net_AddHeader(const wchar_t* format, ...)
{
    wchar_t buf[256];

    va_list va;
    va_start(va, format);
    NET_VSNPRINTFW(buf, format, va);
    va_end(va);

    Net_AppendHeader(buf);
}

void Net_TerminateHeader()
{
    StringCchCatExW(net_headers_ptr, net_headers_rem, L"\r\n", &net_headers_ptr, &net_headers_rem, 0);
}

// Read the headers in the current response.
bool Net_ReadHeader(const wchar_t* header, wchar_t* dest, int32_t dest_size)
{
    assert(net_cur_response); // Can only be called in format_response_func.
    return Net_QueryHttpHeader(net_cur_response, header, dest, dest_size);
}

NetAPIResponse* Net_GetResponseFromHandle(int32_t response_handle, NetAPIDesc* type_check)
{
    NetAPIResponse* response = (NetAPIResponse*)response_handle;

    if (response)
    {
        if (type_check)
        {
            if (response->desc != type_check)
            {
                return NULL;
            }
        }
    }

    return response;
}

int32_t Net_MakeResponseHandle(NetAPIResponse* response)
{
    assert(response->status); // Must only be called on a valid response.
    return (int32_t)response;
}

bool Net_InitAuth()
{
    char path[256];
    smutils->BuildPath(Path_SM, path, sizeof(path), "data/replay_viewer/auth.txt");

    char* token = Net_ReadFileAsString(path);

    if (token == NULL)
    {
        smutils->LogError(myself, "ERROR: Could not load auth file");
        return false;
    }

    Net_ToUTF16(token, strlen(token), net_auth_token, NET_ARRAY_SIZE(net_auth_token));

    Net_Free(token);

    return true;
}

bool Net_InitInet()
{
    // Open in synchronous mode because we intend to only call from a worker thread.
    net_inet_h = InternetOpenW(L"REPLAYVIEWER", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

    if (net_inet_h == NULL)
    {
        DWORD error = GetLastError();
        smutils->LogError(myself, "ERROR: Could not initialize INET, starting in offline mode (%lu)", error);
        return false;
    }

    net_session_h = InternetConnectW(net_inet_h, NET_HOST, NET_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, INTERNET_NO_CALLBACK);

    if (net_session_h == NULL)
    {
        DWORD error = GetLastError();
        smutils->LogError(myself, "ERROR: Could not create a session to host (%lu)", error);
        return false;
    }

    net_response_buffer.reserve(1024);

    return true;
}

// Used to poll completion of the network requests from the net thread.
void Net_ReadThreadResponses()
{
    // Must operate on the response outside the lock, because any API handler
    // will probably be calling into the script code, so we should not block too long.

    NET_LOCK(&net_response_lock);
    net_local_responses = std::move(net_responses);
    NET_UNLOCK(&net_response_lock);

    for (size_t i = 0; i < net_local_responses.size(); i++)
    {
        NetAPIResponse* response = &net_local_responses[i];

        // Still call even if the response failed, in order to call script functions.
        response->desc->handle_response_func(response);
        response->desc->free_response_func(response); // Not needed any more.
    }

    net_local_responses.clear();
}

void Net_Update(bool simulating)
{
    Net_ReadThreadResponses();
}

bool Net_InitThread()
{
    net_wake_event_h = CreateEventW(NULL, FALSE, FALSE, NULL);
    net_thread_h = CreateThread(NULL, 0, Net_ThreadProc, NULL, 0, NULL);

    return true;
}

bool Net_ConnectedToInet()
{
    if (InternetAttemptConnect(0) != 0)
    {
        return false;
    }

    return true;
}

cell_t Net_ConnectedToInet(IPluginContext* context, const cell_t* params)
{
    return Net_ConnectedToInet();
}

cell_t Net_CreateDummyNavForMap(IPluginContext* context, const cell_t* params)
{
    const char* map = gamehelpers->GetCurrentMap();

    char* source_ptr;
    context->LocalToString(params[1], &source_ptr);

    char source_path_buf[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, source_path_buf, NET_ARRAY_SIZE(source_path_buf), source_ptr);

    char dest_ptr[PLATFORM_MAX_PATH];
    NET_SNPRINTF(dest_ptr, "maps\\%s.nav", map);

    char dest_path_buf[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, dest_path_buf, NET_ARRAY_SIZE(dest_path_buf), dest_ptr);

    CopyFileA(source_path_buf, dest_path_buf, TRUE); // Don't need to overwrite if something exists already.

    return 0;
}

bool Net_Init()
{
    if (!Net_InitInet())
    {
        return false;
    }

    if (!Net_InitAuth())
    {
        return false;
    }

    if (!Net_InitThread())
    {
        return false;
    }

    return true;
}

void Net_AllLoaded()
{
    // Used to poll completion of the network requests.
    smutils->AddGameFrameHook(Net_Update);

    extern sp_nativeinfo_t NET_NATIVES[];
    sharesys->AddNatives(myself, NET_NATIVES); // Add our own natives.

    for (size_t i = 0; i < NET_NUM_API_DESCS; i++)
    {
        NetAPIDesc* desc = NET_API_DESCS[i];
        desc->init_func();
    }
}

void Net_Shutdown()
{
    smutils->RemoveGameFrameHook(Net_Update);

    for (size_t i = 0; i < NET_NUM_API_DESCS; i++)
    {
        NetAPIDesc* desc = NET_API_DESCS[i];
        desc->free_func();
    }

    if (net_inet_h)
    {
        InternetCloseHandle(net_inet_h);
        net_inet_h = NULL;
    }

    if (net_thread_h)
    {
        NET_LOCK(&net_request_lock);
        net_requests.clear();
        NET_UNLOCK(&net_request_lock);

        SetEvent(net_wake_event_h);

        WaitForSingleObject(net_thread_h, INFINITE);
        CloseHandle(net_thread_h);
        net_thread_h = NULL;
    }

    if (net_wake_event_h)
    {
        CloseHandle(net_wake_event_h);
        net_wake_event_h = NULL;
    }

    if (net_session_h)
    {
        InternetCloseHandle(net_session_h);
        net_session_h = NULL;
    }
}

sp_nativeinfo_t NET_NATIVES[] = {
    sp_nativeinfo_t { "Net_ConnectedToInet", Net_ConnectedToInet },
    sp_nativeinfo_t { "Net_CreateDummyNavForMap", Net_CreateDummyNavForMap },
    sp_nativeinfo_t { NULL, NULL },
};
