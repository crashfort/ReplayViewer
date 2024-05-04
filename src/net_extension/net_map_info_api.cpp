#include "net_priv.h"

// Download map info from server.

extern NetAPIDesc NET_MAP_INFO_API_DESC;

struct NetMapInfoAPIResponse
{
    int32_t num_stages;
    int32_t num_bonuses;
    int32_t linear;
};

// Thse are used to give data back to the script.
// Can only be used in the main thread.
IForward* net_map_info_dl_received;
IForward* net_map_info_dl_failed;

void Net_InitMapInfoAPI()
{
    extern sp_nativeinfo_t NET_MAP_INFO_API_NATIVES[];
    sharesys->AddNatives(myself, NET_MAP_INFO_API_NATIVES);

    net_map_info_dl_received = forwards->CreateForward("Net_MapInfoDownloadReceived", ET_Event, 1, NULL, Param_Cell);
    net_map_info_dl_failed = forwards->CreateForward("Net_MapInfoDownloadFailed", ET_Event, 0, NULL);
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

    wchar_t stupid_map[128];
    NET_TO_UTF16(map, stupid_map);

    wchar_t req_string[128];
    NET_SNPRINTFW(req_string, L"/map/%s/info", stupid_map);

    Net_MakeHttpRequest(&NET_MAP_INFO_API_DESC, req_string, NULL);

    return 1;
}

// Called in the net thread to format the input bytes into a response structure.
void* Net_FormatMapInfoResponse(void* input, int32_t input_size, NetAPIRequest* request)
{
    // We get a JSON response.

    NetMapInfoAPIResponse* response_state = NULL;

    json_value_s* root = json_parse((const char*)input, sizeof(char) * input_size);

    if (root == NULL)
    {
        goto rfail; // TODO Need to log the error but we cannot use the SM logs because we are not in the main thread here.
    }

    json_object_s* root_val = json_value_as_object(root);

    json_value_s* linear_val = NULL;
    json_value_s* num_stages_val = NULL;
    json_value_s* num_bonuses_val = NULL;

    NetJsonFindPair needed_values[] = {
        NetJsonFindPair { "isLinear", &linear_val },
        NetJsonFindPair { "cpCount", &num_stages_val },
        NetJsonFindPair { "bCount", &num_bonuses_val },
    };

    size_t num_found = Net_FindJsonValuesInObject(root_val, needed_values, NET_ARRAY_SIZE(needed_values));

    if (num_found < NET_ARRAY_SIZE(needed_values))
    {
        goto rfail;
    }

    response_state = NET_ZALLOC(NetMapInfoAPIResponse);
    response_state->num_stages = atoi(Net_GetJsonString(num_stages_val));
    response_state->num_bonuses = atoi(Net_GetJsonString(num_bonuses_val));
    response_state->linear = Net_GetJsonBool(linear_val);

    goto rexit;

rfail:
    if (root)
    {
        free(root);
        root = NULL;
    }

rexit:
    return response_state;
}

// Called in the main thread to do something with the response.
void Net_HandleMapInfoResponse(NetAPIResponse* response)
{
    NetMapInfoAPIResponse* response_state = (NetMapInfoAPIResponse*)response->response_state;

    if (response->status)
    {
        net_map_info_dl_received->PushCell(Net_MakeResponseHandle(response)); // Give handle to script.
        net_map_info_dl_received->Execute();
    }

    else
    {
        net_map_info_dl_failed->Execute();
    }
}

void Net_FreeMapInfoResponse(NetAPIResponse* response)
{
    NetMapInfoAPIResponse* response_state = (NetMapInfoAPIResponse*)response->response_state;

    if (response_state)
    {
        Net_Free(response_state);
    }
}

cell_t Net_MapInfoGetNumStages(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_MAP_INFO_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    NetMapInfoAPIResponse* response_state = (NetMapInfoAPIResponse*)response->response_state;
    return response_state->num_stages;
}

cell_t Net_MapInfoGetNumBonuses(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_MAP_INFO_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    NetMapInfoAPIResponse* response_state = (NetMapInfoAPIResponse*)response->response_state;
    return response_state->num_bonuses;
}

cell_t Net_MapInfoGetLinear(IPluginContext* context, const cell_t* params)
{
    NetAPIResponse* response = Net_GetResponseFromHandle(params[1], &NET_MAP_INFO_API_DESC);

    if (response == NULL)
    {
        context->ReportError("Invalid handle");
        return 0;
    }

    NetMapInfoAPIResponse* response_state = (NetMapInfoAPIResponse*)response->response_state;
    return response_state->linear;
}

sp_nativeinfo_t NET_MAP_INFO_API_NATIVES[] = {
    sp_nativeinfo_t { "Net_DownloadMapInfo", Net_DownloadMapInfo },
    sp_nativeinfo_t { "Net_MapInfoGetNumStages", Net_MapInfoGetNumStages },
    sp_nativeinfo_t { "Net_MapInfoGetNumBonuses", Net_MapInfoGetNumBonuses },
    sp_nativeinfo_t { "Net_MapInfoGetLinear", Net_MapInfoGetLinear },
    sp_nativeinfo_t { NULL, NULL },
};

NetAPIDesc NET_MAP_INFO_API_DESC = NetAPIDesc {
    Net_InitMapInfoAPI,
    Net_FreeMapInfoAPI,
    Net_FormatMapInfoResponse,
    Net_HandleMapInfoResponse,
    Net_FreeMapInfoResponse,
    NULL,
};
