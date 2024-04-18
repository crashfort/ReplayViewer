#include "net_priv.h"

// Download map info from server.

extern NetAPIDesc NET_MAP_INFO_API_DESC;

struct NetMapInfoAPIRequest
{
    int32_t user_id;
};

struct NetMapInfoAPIResponse
{
    int32_t num_stages;
    int32_t num_bonuses;
};

// Thse are used to give data back to the script.
// Can only be used in the main thread.
IForward* net_map_info_dl_received;
IForward* net_map_info_dl_failed;

void Net_InitMapInfoAPI()
{
    extern sp_nativeinfo_t NET_MAP_INFO_API_NATIVES[];
    sharesys->AddNatives(myself, NET_MAP_INFO_API_NATIVES);

    net_map_info_dl_received = forwards->CreateForward("Net_MapInfoDownloadReceived", ET_Event, 2, NULL, Param_Cell, Param_Cell);
    net_map_info_dl_failed = forwards->CreateForward("Net_MapInfoDownloadFailed", ET_Event, 1, NULL, Param_Cell);
}

void Net_FreeMapInfoAPI()
{
    forwards->ReleaseForward(net_map_info_dl_received);
    forwards->ReleaseForward(net_map_info_dl_failed);
}

// Entrypoint for API.
cell_t Net_DownloadMapInfo(IPluginContext* context, const cell_t* params)
{
    if (!Net_ConnectedToInet())
    {
        return 0;
    }

    const char* map = gamehelpers->GetCurrentMap();

    NetMapInfoAPIRequest* request_state = NET_ZALLOC(NetMapInfoAPIRequest);
    request_state->user_id = params[1];

    // TODO Don't know the input path.
    wchar_t req_string[128];
    NET_SNPRINTFW(req_string, L"");

    Net_MakeHttpRequest(&NET_MAP_INFO_API_DESC, req_string, request_state);

    return 1;
}

// Called in the net thread to format the input bytes into a response structure.
void* Net_FormatMapInfoResponse(void* input, int32_t input_size, NetAPIRequest* request)
{
    NetMapInfoAPIResponse* response_state = NET_ZALLOC(NetMapInfoAPIResponse);

    // TODO Don't know the response format.

    return response_state;
}

// Called in the main thread to do something with the response.
void Net_HandleMapInfoResponse(NetAPIResponse* response)
{
    NetMapInfoAPIRequest* request_state = (NetMapInfoAPIRequest*)response->request_state;
    NetMapInfoAPIResponse* response_state = (NetMapInfoAPIResponse*)response->response_state;

    if (response->status)
    {
        net_map_info_dl_received->PushCell(request_state->user_id);
        net_map_info_dl_received->PushCell(Net_MakeResponseHandle(response)); // Give handle to script.
        net_map_info_dl_received->Execute();
    }

    else
    {
        net_map_info_dl_failed->PushCell(request_state->user_id);
        net_map_info_dl_failed->Execute();
    }
}

// Called when the script calls Net_CloseHandle on the handle from Net_MakeResponseHandle or automatically by the net state when the response fails.
void Net_FreeMapInfoResponse(NetAPIResponse* response)
{
    NetMapInfoAPIRequest* request_state = (NetMapInfoAPIRequest*)response->request_state;
    NetMapInfoAPIResponse* response_state = (NetMapInfoAPIResponse*)response->response_state;

    if (response_state)
    {
        Net_Free(response_state);
    }

    if (request_state)
    {
        Net_Free(request_state);
    }
}

// Called by the script to read the response data.
cell_t Net_MapInfoGetNumStages(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1]);

    if (response == NULL)
    {
        return 0;
    }

    if (response->desc != &NET_MAP_INFO_API_DESC)
    {
        context->ReportError("Handle type mismatch");
        return 0;
    }

    NetMapInfoAPIResponse* response_state = (NetMapInfoAPIResponse*)response->response_state;
    return response_state->num_stages;
}

// Called by the script to read the response data.
cell_t Net_MapInfoGetNumBonuses(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1]);

    if (response == NULL)
    {
        return 0;
    }

    if (response->desc != &NET_MAP_INFO_API_DESC)
    {
        context->ReportError("Handle type mismatch");
        return 0;
    }

    NetMapInfoAPIResponse* response_state = (NetMapInfoAPIResponse*)response->response_state;
    return response_state->num_bonuses;
}

sp_nativeinfo_t NET_MAP_INFO_API_NATIVES[] = {
    sp_nativeinfo_t { "Net_DownloadMapInfo", Net_DownloadMapInfo },
    sp_nativeinfo_t { "Net_MapInfoGetNumStages", Net_MapInfoGetNumStages },
    sp_nativeinfo_t { "Net_MapInfoGetNumBonuses", Net_MapInfoGetNumBonuses },
    sp_nativeinfo_t { NULL, NULL },
};

NetAPIDesc NET_MAP_INFO_API_DESC = NetAPIDesc {
    Net_InitMapInfoAPI,
    Net_FreeMapInfoAPI,
    Net_FormatMapInfoResponse,
    Net_HandleMapInfoResponse,
    Net_FreeMapInfoResponse,
};
