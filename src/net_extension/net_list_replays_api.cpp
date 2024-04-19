#include "net_priv.h"

// Download replay list from server.

extern NetAPIDesc NET_REPLAY_LIST_API_DESC;

struct NetReplayListAPIRequest
{
    int32_t user_id;
    int32_t zone_id;
    int32_t angle_type;
};

struct NetReplayListing
{
    char name[512];
    int32_t time;
};

struct NetReplayListAPIResponse
{
    NetReplayListing* listings;
    int32_t num_listings;
};

// Thse are used to give data back to the script.
// Can only be used in the main thread.
IForward* net_replay_list_dl_received;
IForward* net_replay_list_dl_failed;

void Net_InitReplayListAPI()
{
    extern sp_nativeinfo_t NET_REPLAY_LIST_API_NATIVES[];
    sharesys->AddNatives(myself, NET_REPLAY_LIST_API_NATIVES);

    net_replay_list_dl_received = forwards->CreateForward("Net_ReplayListDownloadReceived", ET_Event, 1, NULL, Param_Cell);
    net_replay_list_dl_failed = forwards->CreateForward("Net_ReplayListDownloadFailed", ET_Event, 0, NULL);
}

void Net_FreeReplayListAPI()
{
    forwards->ReleaseForward(net_replay_list_dl_received);
    forwards->ReleaseForward(net_replay_list_dl_failed);
}

// Entrypoint for API.
cell_t Net_DownloadReplayList(IPluginContext* context, const cell_t* params)
{
    if (!Net_ConnectedToInet())
    {
        return 0;
    }

    const char* map = gamehelpers->GetCurrentMap();

    NetReplayListAPIRequest* request_state = NET_ZALLOC(NetReplayListAPIRequest);
    request_state->user_id = params[1];
    request_state->zone_id = params[2];
    request_state->angle_type = params[3];

    // TODO Don't know the input path.
    wchar_t req_string[128];
    NET_SNPRINTFW(req_string, L"");

    Net_MakeHttpRequest(&NET_REPLAY_LIST_API_DESC, req_string, request_state);

    return 1;
}

// Called in the net thread to format the input bytes into a response structure.
void* Net_FormatReplayListResponse(void* input, int32_t input_size, NetAPIRequest* request)
{
    NetReplayListAPIResponse* response_state = NET_ZALLOC(NetReplayListAPIResponse);

    // TODO Don't know the response format.

    return response_state;
}

// Called in the main thread to do something with the response.
void Net_HandleReplayListResponse(NetAPIResponse* response)
{
    NetReplayListAPIRequest* request_state = (NetReplayListAPIRequest*)response->request_state;
    NetReplayListAPIResponse* response_state = (NetReplayListAPIResponse*)response->response_state;

    if (response->status)
    {
        net_replay_list_dl_received->PushCell(Net_MakeResponseHandle(response)); // Give handle to script.
        net_replay_list_dl_received->Execute();
    }

    else
    {
        net_replay_list_dl_failed->Execute();
    }
}

// Called when the script calls Net_CloseHandle on the handle from Net_MakeResponseHandle or automatically by the net state when the response fails.
void Net_FreeReplayListResponse(NetAPIResponse* response)
{
    NetReplayListAPIRequest* request_state = (NetReplayListAPIRequest*)response->request_state;
    NetReplayListAPIResponse* response_state = (NetReplayListAPIResponse*)response->response_state;

    if (response_state)
    {
        if (response_state->listings)
        {
            Net_Free(response_state->listings);
        }

        Net_Free(response_state);
    }

    if (request_state)
    {
        Net_Free(request_state);
    }
}

cell_t Net_ReplayListGetUserId(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_REPLAY_LIST_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    NetReplayListAPIRequest* request_state = (NetReplayListAPIRequest*)response->request_state;
    return request_state->user_id;
}

cell_t Net_ReplayListGetNum(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_REPLAY_LIST_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    NetReplayListAPIResponse* response_state = (NetReplayListAPIResponse*)response->response_state;
    return response_state->num_listings;
}

cell_t Net_ReplayListGetPlayerName(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_REPLAY_LIST_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    NetReplayListAPIResponse* response_state = (NetReplayListAPIResponse*)response->response_state;

    if (!Net_IdxInRange(params[2], response_state->num_listings))
    {
        context->ReportError("Index out of range (passed %d, max is %d)", params[2], response_state->num_listings);
        return 0;
    }

    NetReplayListing* listing = &response_state->listings[params[2]];

    char* dest_ptr;
    context->LocalToString(params[3], &dest_ptr);

    StringCchCopyA(dest_ptr, params[4], listing->name);

    return 1;
}

cell_t Net_ReplayListGetTime(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_REPLAY_LIST_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    NetReplayListAPIResponse* response_state = (NetReplayListAPIResponse*)response->response_state;

    if (!Net_IdxInRange(params[2], response_state->num_listings))
    {
        context->ReportError("Index out of range (passed %d, max is %d)", params[2], response_state->num_listings);
        return 0;
    }

    NetReplayListing* listing = &response_state->listings[params[2]];
    return listing->time;
}

cell_t Net_ReplayListGetZoneId(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_REPLAY_LIST_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    NetReplayListAPIRequest* request_state = (NetReplayListAPIRequest*)response->request_state;
    return request_state->zone_id;
}

cell_t Net_ReplayListGetAngleType(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_REPLAY_LIST_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    NetReplayListAPIRequest* request_state = (NetReplayListAPIRequest*)response->request_state;
    return request_state->angle_type;
}

sp_nativeinfo_t NET_REPLAY_LIST_API_NATIVES[] = {
    sp_nativeinfo_t { "Net_DownloadReplayList", Net_DownloadReplayList },
    sp_nativeinfo_t { "Net_ReplayListGetUserId", Net_ReplayListGetUserId },
    sp_nativeinfo_t { "Net_ReplayListGetNum", Net_ReplayListGetNum },
    sp_nativeinfo_t { "Net_ReplayListGetPlayerName", Net_ReplayListGetPlayerName },
    sp_nativeinfo_t { "Net_ReplayListGetTime", Net_ReplayListGetTime },
    sp_nativeinfo_t { "Net_ReplayListGetZoneId", Net_ReplayListGetZoneId },
    sp_nativeinfo_t { "Net_ReplayListGetAngleType", Net_ReplayListGetAngleType },
    sp_nativeinfo_t { NULL, NULL },
};

NetAPIDesc NET_REPLAY_LIST_API_DESC = NetAPIDesc {
    Net_InitReplayListAPI,
    Net_FreeReplayListAPI,
    Net_FormatReplayListResponse,
    Net_HandleReplayListResponse,
    Net_FreeReplayListResponse,
};
