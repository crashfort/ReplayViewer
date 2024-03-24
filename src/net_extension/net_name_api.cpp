#include "net_priv.h"

// API to get the name of a player from an internal player id.

struct NetNameAPIRequest
{
    int player_id;
};

struct NetNameAPIResponse
{
    char name[256];
};

// Thse are used to give data back to the script.
// Can only be used in the main thread.
IForward* net_player_name_received;
IForward* net_player_name_request_failed;

void Net_InitNameAPI()
{
    extern sp_nativeinfo_t NET_NAME_API_NATIVES[];
    sharesys->AddNatives(myself, NET_NAME_API_NATIVES);

    net_player_name_received = forwards->CreateForward("Net_PlayerNameReceived", ET_Event, 2, NULL, Param_Cell, Param_String);
    net_player_name_request_failed = forwards->CreateForward("Net_PlayerNameRequestFailed", ET_Event, 1, NULL, Param_Cell);
}

void Net_FreeNameAPI()
{
    forwards->ReleaseForward(net_player_name_received);
    forwards->ReleaseForward(net_player_name_request_failed);
}

// Entrypoint for API.
cell_t Net_RequestPlayerName(IPluginContext* pContext, const cell_t* params)
{
    if (!Net_ConnectedToInet())
    {
        return 0;
    }

    NetNameAPIRequest* req_state = (NetNameAPIRequest*)malloc(sizeof(NetNameAPIRequest));
    *req_state = {};

    req_state->player_id = params[1];

    // TODO Don't know the input path.
    wchar_t req_string[128];
    NET_SNPRINTFW(req_string, L"/api/get-player-name/%d", req_state->player_id);

    Net_MakeHttpRequest(NET_API_GET_PLAYER_NAME, req_string, req_state);

    return 1;
}

void Net_FormatNameResponse(void* input, int input_size, void* dest)
{
    NetNameAPIResponse* data = (NetNameAPIResponse*)dest;

    // TODO Don't know the response format.
}

void Net_HandleNameResponse(NetAPIResponse* response)
{
    NetNameAPIResponse* data = (NetNameAPIResponse*)response->data;
    NetNameAPIRequest* req_state = (NetNameAPIRequest*)response->request_state;

    if (response->status)
    {
        net_player_name_received->PushCell(req_state->player_id);
        net_player_name_received->PushString(data->name);
        net_player_name_received->Execute();
    }

    else
    {
        net_player_name_request_failed->PushCell(req_state->player_id);
        net_player_name_request_failed->Execute();
    }
}

sp_nativeinfo_t NET_NAME_API_NATIVES[] = {
    sp_nativeinfo_t { "Net_RequestPlayerName", Net_RequestPlayerName },
    sp_nativeinfo_t { NULL, NULL },
};

NetAPIDesc NET_NAME_API_DESC = NetAPIDesc {
    NET_API_GET_PLAYER_NAME,
    sizeof(NetNameAPIResponse),
    Net_InitNameAPI,
    Net_FreeNameAPI,
    Net_FormatNameResponse,
    Net_HandleNameResponse,
    NULL,
};
