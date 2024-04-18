#include "net_priv.h"

// Download replays from server.

extern NetAPIDesc NET_REPLAY_DOWNLOAD_API_DESC;

struct NetReplayDownloadAPIRequest
{
    int32_t zone_id;
    int32_t angle_type;
    int32_t index;
};

struct NetReplayDownloadAPIResponse
{
    uint8_t* bytes; // File data.
    int32_t size; // Size of file.
    int32_t read_pos; // How much the script has read.
};

// Thse are used to give data back to the script.
// Can only be used in the main thread.
IForward* net_replay_dl_received;
IForward* net_replay_dl_failed;

void Net_InitReplayDownloadAPI()
{
    extern sp_nativeinfo_t NET_REPLAY_DOWNLOAD_API_NATIVES[];
    sharesys->AddNatives(myself, NET_REPLAY_DOWNLOAD_API_NATIVES);

    net_replay_dl_received = forwards->CreateForward("Net_ReplayDownloadReceived", ET_Event, 1, NULL, Param_Cell);
    net_replay_dl_failed = forwards->CreateForward("Net_ReplayDownloadFailed", ET_Event, 0, NULL);
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
    request_state->zone_id = params[1];
    request_state->angle_type = params[2];
    request_state->index = params[3];

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

bool Net_CanReadReplayStream(NetReplayDownloadAPIResponse* response_state, int32_t size)
{
    int32_t remaining_space = response_state->size - response_state->read_pos;
    return size <= remaining_space;
}

// Read some bytes from the downloaded file.
bool Net_ReadReplayStream(NetReplayDownloadAPIResponse* response_state, void* dest, int32_t size)
{
    if (!Net_CanReadReplayStream(response_state, size))
    {
        return false;
    }

    memcpy(dest, response_state->bytes + response_state->read_pos, size);

    response_state->read_pos += size; // Advance reading position.

    return true;
}

// Called by the script to read the response data.
cell_t Net_ReplayDownloadReadData(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_REPLAY_DOWNLOAD_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    NetReplayDownloadAPIResponse* response_state = (NetReplayDownloadAPIResponse*)response->response_state;

    if (response_state->read_pos == response_state->size)
    {
        return 0; // Nothing more to read.
    }

    cell_t* dest;
    context->LocalToPhysAddr(params[2], &dest);

    bool res = false;

    // Hella messy because SourcePawn uses a generic cell type which is inconsistent and weird about its usage.
    // The logic about the sizes here is from the SourceMod ReadFile function.

    switch (params[4])
    {
        case 4:
        {
            res = Net_ReadReplayStream(response_state, dest, sizeof(cell_t) * params[3]);
            break;
        }

        case 2:
        {
            for (cell_t i = 0; i < params[3]; i++)
            {
                uint16_t val;
                res = Net_ReadReplayStream(response_state, &val, sizeof(uint16_t));

                if (!res)
                {
                    break;
                }

                dest[i] = val;
            }

            break;
        }

        case 1:
        {
            for (cell_t i = 0; i < params[3]; i++)
            {
                uint8_t val;
                res = Net_ReadReplayStream(response_state, &val, sizeof(uint8_t));

                if (!res)
                {
                    break;
                }

                dest[i] = val;
            }

            break;
        }

        default:
        {
            context->ReportError("Invalid size specifier (%d is not 1, 2, or 4)", params[4]);
            return 0;
        }
    }

    return res;
}

cell_t Net_ReplayDownloadRewind(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_REPLAY_DOWNLOAD_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    NetReplayDownloadAPIResponse* response_state = (NetReplayDownloadAPIResponse*)response->response_state;
    response_state->read_pos = 0;

    return 1;
}

sp_nativeinfo_t NET_REPLAY_DOWNLOAD_API_NATIVES[] = {
    sp_nativeinfo_t { "Net_DownloadReplay", Net_DownloadReplay },
    sp_nativeinfo_t { "Net_ReplayDownloadReadData", Net_ReplayDownloadReadData },
    sp_nativeinfo_t { "Net_ReplayDownloadRewind", Net_ReplayDownloadRewind },
    sp_nativeinfo_t { NULL, NULL },
};

NetAPIDesc NET_REPLAY_DOWNLOAD_API_DESC = NetAPIDesc {
    Net_InitReplayDownloadAPI,
    Net_FreeReplayDownloadAPI,
    Net_FormatReplayDownloadResponse,
    Net_HandleReplayDownloadResponse,
    Net_FreeReplayDownloadResponse,
};
