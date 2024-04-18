#include "net_priv.h"

// Download replays from server.

extern NetAPIDesc NET_REPLAY_DOWNLOAD_API_DESC;

struct NetReplayDownloadAPIRequest
{
    int32_t user_id;
    int32_t zone_id;
    int32_t angle_type;
    int32_t index;
};

struct NetReplayDownloadAPIResponse
{
    uint8_t* bytes; // File data.
    int32_t size; // Size of file.
};

// Thse are used to give data back to the script.
// Can only be used in the main thread.
IForward* net_replay_dl_received;
IForward* net_replay_dl_failed;

void Net_InitReplayDownloadAPI()
{
    extern sp_nativeinfo_t NET_REPLAY_DOWNLOAD_API_NATIVES[];
    sharesys->AddNatives(myself, NET_REPLAY_DOWNLOAD_API_NATIVES);

    net_replay_dl_received = forwards->CreateForward("Net_ReplayDownloadReceived", ET_Event, 2, NULL, Param_Cell, Param_Cell);
    net_replay_dl_failed = forwards->CreateForward("Net_ReplayDownloadFailed", ET_Event, 1, NULL, Param_Cell);
}

void Net_FreeReplayDownloadAPI()
{
    forwards->ReleaseForward(net_replay_dl_received);
    forwards->ReleaseForward(net_replay_dl_failed);
}

// Entrypoint for API.
cell_t Net_DownloadReplay(IPluginContext* context, const cell_t* params)
{
    if (!Net_ConnectedToInet())
    {
        return 0;
    }

    const char* map = gamehelpers->GetCurrentMap();

    NetReplayDownloadAPIRequest* request_state = NET_ZALLOC(NetReplayDownloadAPIRequest);
    request_state->user_id = params[1];
    request_state->zone_id = params[2];
    request_state->angle_type = params[3];
    request_state->index = params[4];

    // TODO Don't know the input path.
    wchar_t req_string[128];
    NET_SNPRINTFW(req_string, L"");

    Net_MakeHttpRequest(&NET_REPLAY_DOWNLOAD_API_DESC, req_string, request_state);

    return 1;
}

// Called in the net thread to format the input bytes into a response structure.
void* Net_FormatReplayDownloadResponse(void* input, int32_t input_size, NetAPIRequest* request)
{
    NetReplayDownloadAPIResponse* response_state = NET_ZALLOC(NetReplayDownloadAPIResponse);

    // TODO Don't know the response format.

    return response_state;
}

// Called in the main thread to do something with the response.
void Net_HandleReplayDownloadResponse(NetAPIResponse* response)
{
    NetReplayDownloadAPIRequest* request_state = (NetReplayDownloadAPIRequest*)response->request_state;
    NetReplayDownloadAPIResponse* response_state = (NetReplayDownloadAPIResponse*)response->response_state;

    if (response->status)
    {
        net_replay_dl_received->PushCell(request_state->user_id);
        net_replay_dl_received->PushCell(Net_MakeResponseHandle(response)); // Give handle to script.
        net_replay_dl_received->Execute();
    }

    else
    {
        net_replay_dl_failed->PushCell(request_state->user_id);
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

// Called by the script to read the response data.
cell_t Net_ReplayDownloadGetData(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1]);

    if (response == NULL)
    {
        return 0;
    }

    if (response->desc != &NET_REPLAY_DOWNLOAD_API_DESC)
    {
        context->ReportError("Handle type mismatch");
        return 0;
    }

    NetReplayDownloadAPIResponse* response_state = (NetReplayDownloadAPIResponse*)response->response_state;

    return 1;
}

sp_nativeinfo_t NET_REPLAY_DOWNLOAD_API_NATIVES[] = {
    sp_nativeinfo_t { "Net_DownloadReplay", Net_DownloadReplay },
    sp_nativeinfo_t { "Net_ReplayDownloadGetData", Net_ReplayDownloadGetData },
    sp_nativeinfo_t { NULL, NULL },
};

NetAPIDesc NET_REPLAY_DOWNLOAD_API_DESC = NetAPIDesc {
    Net_InitReplayDownloadAPI,
    Net_FreeReplayDownloadAPI,
    Net_FormatReplayDownloadResponse,
    Net_HandleReplayDownloadResponse,
    Net_FreeReplayDownloadResponse,
};
