#include "net_priv.h"

// Download replays from server.

// Maximum size a decompressed replay can be.
const int32_t NET_REPLAY_DL_DECOMPRESS_SIZE = 4 * 1024 * 1024;

extern NetAPIDesc NET_REPLAY_DOWNLOAD_API_DESC;

struct NetReplayDownloadAPIRequest
{
    int32_t user_id;
    int32_t zone_id;
    int32_t angle_type;
    int32_t rank;
};

struct NetReplayDownloadAPIResponse
{
    void* bytes; // File data.
    int32_t size; // Size of file.
};

// Thse are used to give data back to the script.
// Can only be used in the main thread.
IForward* net_replay_dl_received;
IForward* net_replay_dl_failed;

void* net_replay_dl_decompress_buf;

void Net_InitReplayDownloadAPI()
{
    extern sp_nativeinfo_t NET_REPLAY_DOWNLOAD_API_NATIVES[];
    sharesys->AddNatives(myself, NET_REPLAY_DOWNLOAD_API_NATIVES);

    net_replay_dl_received = forwards->CreateForward("Net_ReplayDownloadReceived", ET_Event, 1, NULL, Param_Cell);
    net_replay_dl_failed = forwards->CreateForward("Net_ReplayDownloadFailed", ET_Event, 0, NULL);

    net_replay_dl_decompress_buf = Net_Alloc(NET_REPLAY_DL_DECOMPRESS_SIZE);
}

void Net_FreeReplayDownloadAPI()
{
    forwards->ReleaseForward(net_replay_dl_received);
    forwards->ReleaseForward(net_replay_dl_failed);
    Net_Free(net_replay_dl_decompress_buf);
}

// Entrypoint for API.
cell_t Net_DownloadReplay(IPluginContext* context, const cell_t* params)
{
    if (!Net_ConnectedToInet())
    {
        return 0;
    }

    const char* map = gamehelpers->GetCurrentMap();

    wchar_t stupid_map[256];
    NET_TO_UTF16(map, stupid_map);

    NetReplayDownloadAPIRequest* request_state = NET_ZALLOC(NetReplayDownloadAPIRequest);
    request_state->user_id = params[1];
    request_state->zone_id = params[2];
    request_state->angle_type = params[3];
    request_state->rank = params[4];

    wchar_t req_string[256];
    NET_SNPRINTFW(req_string, L"/replay/map/%s/zone/%d/rank/%d/%d", stupid_map, request_state->zone_id, request_state->rank, request_state->angle_type);

    Net_MakeHttpRequest(&NET_REPLAY_DOWNLOAD_API_DESC, req_string, request_state);

    return 1;
}

// Called in the net thread to format the input bytes into a response structure.
void* Net_FormatReplayDownloadResponse(void* input, int32_t input_size, NetAPIRequest* request)
{
    // The data we get here is compressed, so we need to decompress it.

    // TODO There needs to be a header that gives the name of the file.
#if 0
    wchar_t header_value[1024];

    if (!Net_ReadHeader(L"Content-Disposition", header_value, NET_ARRAY_SIZE(header_value)))
    {
        return NULL;
    }
#endif

    uint32_t dest_size = NET_REPLAY_DL_DECOMPRESS_SIZE;
    int32_t res = BZ2_bzBuffToBuffDecompress((char*)net_replay_dl_decompress_buf, &dest_size, (char*)input, input_size, 0, 0);

    if (res != BZ_OK)
    {
        // TODO Need to log the error but we cannot use the SM logs because we are not in the main thread here.
        return NULL;
    }

    NetReplayDownloadAPIResponse* response_state = NET_ZALLOC(NetReplayDownloadAPIResponse);
    response_state->bytes = Net_Alloc(dest_size);
    response_state->size = dest_size;

    memcpy(response_state->bytes, net_replay_dl_decompress_buf, dest_size);

    return response_state;
}

// Called in the main thread to do something with the response.
void Net_HandleReplayDownloadResponse(NetAPIResponse* response)
{
    NetReplayDownloadAPIRequest* request_state = (NetReplayDownloadAPIRequest*)response->request_state;
    NetReplayDownloadAPIResponse* response_state = (NetReplayDownloadAPIResponse*)response->response_state;

    if (response->status)
    {
        net_replay_dl_received->PushCell(Net_MakeResponseHandle(response)); // Give handle to script.
        net_replay_dl_received->Execute();
    }

    else
    {
        net_replay_dl_failed->Execute();
    }
}

// Called when the script calls Net_CloseHandle on the handle from Net_MakeResponseHandle or automatically by the net state when the response fails.
void Net_FreeReplayDownloadResponse(NetAPIResponse* response)
{
    NetReplayDownloadAPIRequest* request_state = (NetReplayDownloadAPIRequest*)response->request_state;
    NetReplayDownloadAPIResponse* response_state = (NetReplayDownloadAPIResponse*)response->response_state;

    if (response_state)
    {
        if (response_state->bytes)
        {
            Net_Free(response_state->bytes);
        }

        Net_Free(response_state);
    }

    if (request_state)
    {
        Net_Free(request_state);
    }
}

cell_t Net_ReplayDownloadGetUserId(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_REPLAY_DOWNLOAD_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    NetReplayDownloadAPIRequest* request_state = (NetReplayDownloadAPIRequest*)response->request_state;
    return request_state->user_id;
}

cell_t Net_ReplayDownloadWriteToFile(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_REPLAY_DOWNLOAD_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    char* dest_ptr;
    context->LocalToString(params[2], &dest_ptr);

    NetReplayDownloadAPIResponse* response_state = (NetReplayDownloadAPIResponse*)response->response_state;

    char path_buf[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path_buf, NET_ARRAY_SIZE(path_buf), dest_ptr);

    HANDLE h = CreateFileA(path_buf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (h == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    WriteFile(h, response_state->bytes, response_state->size, NULL, NULL);

    CloseHandle(h);

    return 1;
}

cell_t Net_ReplayDownloadGetZoneId(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_REPLAY_DOWNLOAD_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    NetReplayDownloadAPIRequest* request_state = (NetReplayDownloadAPIRequest*)response->request_state;
    return request_state->zone_id;
}

cell_t Net_ReplayDownloadGetAngleType(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_REPLAY_DOWNLOAD_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    NetReplayDownloadAPIRequest* request_state = (NetReplayDownloadAPIRequest*)response->request_state;
    return request_state->angle_type;
}

cell_t Net_ReplayDownloadGetRank(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_REPLAY_DOWNLOAD_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    NetReplayDownloadAPIRequest* request_state = (NetReplayDownloadAPIRequest*)response->request_state;
    return request_state->rank;
}

sp_nativeinfo_t NET_REPLAY_DOWNLOAD_API_NATIVES[] = {
    sp_nativeinfo_t { "Net_DownloadReplay", Net_DownloadReplay },
    sp_nativeinfo_t { "Net_ReplayDownloadGetUserId", Net_ReplayDownloadGetUserId },
    sp_nativeinfo_t { "Net_ReplayDownloadWriteToFile", Net_ReplayDownloadWriteToFile },
    sp_nativeinfo_t { "Net_ReplayDownloadGetZoneId", Net_ReplayDownloadGetZoneId },
    sp_nativeinfo_t { "Net_ReplayDownloadGetAngleType", Net_ReplayDownloadGetAngleType },
    sp_nativeinfo_t { "Net_ReplayDownloadGetRank", Net_ReplayDownloadGetRank },
    sp_nativeinfo_t { NULL, NULL },
};

NetAPIDesc NET_REPLAY_DOWNLOAD_API_DESC = NetAPIDesc {
    Net_InitReplayDownloadAPI,
    Net_FreeReplayDownloadAPI,
    Net_FormatReplayDownloadResponse,
    Net_HandleReplayDownloadResponse,
    Net_FreeReplayDownloadResponse,
    NULL,
};
