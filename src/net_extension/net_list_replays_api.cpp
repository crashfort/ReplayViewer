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
    char name[128];
    char time[128];
    char date[128];
    char id[128];
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

    wchar_t stupid_map[128];
    NET_TO_UTF16(map, stupid_map);

    NetReplayListAPIRequest* request_state = NET_ZALLOC(NetReplayListAPIRequest);
    request_state->user_id = params[1];
    request_state->zone_id = params[2];
    request_state->angle_type = params[3];

    wchar_t req_string[128];
    NET_SNPRINTFW(req_string, L"/replay/map/%s/zone/%d/%d", stupid_map, request_state->zone_id, request_state->angle_type);

    Net_MakeHttpRequest(&NET_REPLAY_LIST_API_DESC, req_string, request_state);

    return 1;
}

// Called in the net thread to format the input bytes into a response structure.
void* Net_FormatReplayListResponse(void* input, int32_t input_size, NetAPIRequest* request)
{
    // We get a JSON response.

    NetReplayListAPIResponse* response_state = NULL;

    json_value_s* root = json_parse((const char*)input, sizeof(char) * input_size);

    if (root == NULL)
    {
        goto rfail; // TODO Need to log the error but we cannot use the SM logs because we are not in the main thread here.
    }

    json_array_s* root_val = json_value_as_array(root);

    // Create one listing per array entry.

    response_state = NET_ZALLOC(NetReplayListAPIResponse);
    response_state->listings = NET_ZALLOC_NUM(NetReplayListing, root_val->length);
    response_state->num_listings = root_val->length;

    int32_t idx = 0;

    for (json_array_element_s* array_elm = root_val->start; array_elm; array_elm = array_elm->next)
    {
        json_object_s* elm_val = json_value_as_object(array_elm->value);

        json_value_s* name_val = NULL;
        json_value_s* time_val = NULL;
        json_value_s* date_val = NULL;
        json_value_s* file_val = NULL;

        NetJsonFindPair needed_values[] = {
            NetJsonFindPair { "player_name", &name_val },
            NetJsonFindPair { "player_time", &time_val },
            NetJsonFindPair { "date", &date_val },
            NetJsonFindPair { "file", &file_val },
        };

        size_t num_found = Net_FindJsonValuesInObject(elm_val, needed_values, NET_ARRAY_SIZE(needed_values));

        if (num_found < NET_ARRAY_SIZE(needed_values))
        {
            goto rfail;
        }

        NetReplayListing* listing = &response_state->listings[idx];
        NET_COPY_STRING(Net_GetJsonString(name_val), listing->name);
        NET_COPY_STRING(Net_GetJsonString(time_val), listing->time);
        NET_COPY_STRING(Net_GetJsonString(date_val), listing->date);
        NET_COPY_STRING(Net_GetJsonString(file_val), listing->id);

        idx++;
    }

    goto rexit;

rfail:
    if (root)
    {
        free(root);
        root = NULL;
    }

    if (response_state)
    {
        Net_Free(response_state);
        response_state = NULL;
    }

rexit:
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

cell_t Net_ReplayListGetId(IPluginContext* context, const cell_t* params)
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

    StringCchCopyA(dest_ptr, params[4], listing->id);

    return 1;
}

cell_t Net_ReplayListGetName(IPluginContext* context, const cell_t* params)
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

    char* dest_ptr;
    context->LocalToString(params[3], &dest_ptr);

    StringCchCopyA(dest_ptr, params[4], listing->time);

    return 1;
}

cell_t Net_ReplayListGetDate(IPluginContext* context, const cell_t* params)
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

    StringCchCopyA(dest_ptr, params[4], listing->date);

    return 1;
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
    sp_nativeinfo_t { "Net_ReplayListGetId", Net_ReplayListGetId },
    sp_nativeinfo_t { "Net_ReplayListGetName", Net_ReplayListGetName },
    sp_nativeinfo_t { "Net_ReplayListGetTime", Net_ReplayListGetTime },
    sp_nativeinfo_t { "Net_ReplayListGetDate", Net_ReplayListGetDate },
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
    NULL,
};
