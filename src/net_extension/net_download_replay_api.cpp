#include "net_priv.h"

// Download replays from server.

extern NetAPIDesc NET_REPLAY_DOWNLOAD_API_DESC;

struct NetReplayDownloadAPIRequest
{
    int rank;
};

struct NetReplayDownloadAPIResponse
{
    char path[256];
};

// Thse are used to give data back to the script.
// Can only be used in the main thread.
IForward* net_replay_dl_received;
IForward* net_replay_dl_failed;

void Net_InitReplayDownloadAPI()
{
    extern sp_nativeinfo_t NET_REPLAY_DOWNLOAD_API_NATIVES[];
    sharesys->AddNatives(myself, NET_REPLAY_DOWNLOAD_API_NATIVES);

    net_replay_dl_received = forwards->CreateForward("Net_ReplayDownloadReceived", ET_Event, 2, NULL, Param_Cell, Param_String);
    net_replay_dl_failed = forwards->CreateForward("Net_ReplayDownloadFailed", ET_Event, 1, NULL, Param_Cell);
}

void Net_FreeReplayDownloadAPI()
{
    forwards->ReleaseForward(net_replay_dl_received);
    forwards->ReleaseForward(net_replay_dl_failed);
}

// Entrypoint for API.
cell_t Net_RequestReplay(IPluginContext* context, const cell_t* params)
{
    if (!Net_ConnectedToInet())
    {
        return 0;
    }

    const char* map = gamehelpers->GetCurrentMap();

    NetReplayDownloadAPIRequest* req_state = (NetReplayDownloadAPIRequest*)malloc(sizeof(NetReplayDownloadAPIRequest));
    *req_state = {};

    req_state->rank = params[1];

    // TODO Don't know the input path.
    wchar_t req_string[128];
    NET_SNPRINTFW(req_string, L"/api/download-replay/%s/%d", map, req_state->rank);

    Net_MakeHttpRequest(&NET_REPLAY_DOWNLOAD_API_DESC, req_string, req_state);

    return 1;
}

bool Net_FormatReplayDownloadResponse(void* input, int input_size, void* dest, NetAPIRequest* request)
{
    NetReplayDownloadAPIResponse* data = (NetReplayDownloadAPIResponse*)dest;

    // TODO Don't know the response format.

    return true;
}

void Net_HandleReplayDownloadResponse(NetAPIResponse* response)
{
    NetReplayDownloadAPIResponse* resp_data = (NetReplayDownloadAPIResponse*)response->data;
    NetReplayDownloadAPIRequest* req_state = (NetReplayDownloadAPIRequest*)response->request_state;

    if (response->status)
    {
        net_replay_dl_received->PushCell(req_state->rank);
        net_replay_dl_received->PushString(resp_data->path);
        net_replay_dl_received->Execute();
    }

    else
    {
        net_replay_dl_failed->PushCell(req_state->rank);
        net_replay_dl_failed->Execute();
    }
}

sp_nativeinfo_t NET_REPLAY_DOWNLOAD_API_NATIVES[] = {
    sp_nativeinfo_t { "Net_RequestReplay", Net_RequestReplay },
    sp_nativeinfo_t { NULL, NULL },
};

NetAPIDesc NET_REPLAY_DOWNLOAD_API_DESC = NetAPIDesc {
    sizeof(NetReplayDownloadAPIResponse),
    Net_InitReplayDownloadAPI,
    Net_FreeReplayDownloadAPI,
    Net_FormatReplayDownloadResponse,
    Net_HandleReplayDownloadResponse,
    NULL,
};
