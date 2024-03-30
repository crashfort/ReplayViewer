#include "net_priv.h"

// Get the name of a player from an internal player id.

extern NetAPIDesc NET_PLAYER_NAME_API_DESC;

struct NetPlayerNameAPIRequest
{
    int player_id;
};

struct NetPlayerNameAPIResponse
{
    char name[256];
};

// Thse are used to give data back to the script.
// Can only be used in the main thread.
IForward* net_player_name_received;
IForward* net_player_name_failed;

void Net_InitPlayerNameAPI()
{
    extern sp_nativeinfo_t NET_PLAYER_NAME_API_NATIVES[];
    sharesys->AddNatives(myself, NET_PLAYER_NAME_API_NATIVES);

    net_player_name_received = forwards->CreateForward("Net_PlayerNameReceived", ET_Event, 2, NULL, Param_Cell, Param_String);
    net_player_name_failed = forwards->CreateForward("Net_PlayerNameFailed", ET_Event, 1, NULL, Param_Cell);
}

void Net_FreePlayerNameAPI()
{
    forwards->ReleaseForward(net_player_name_received);
    forwards->ReleaseForward(net_player_name_failed);
}

// Entrypoint for API.
cell_t Net_RequestPlayerName(IPluginContext* context, const cell_t* params)
{
    if (!Net_ConnectedToInet())
    {
        return 0;
    }

    NetPlayerNameAPIRequest* req_state = (NetPlayerNameAPIRequest*)malloc(sizeof(NetPlayerNameAPIRequest));
    *req_state = {};

    req_state->player_id = params[1];

    // TODO Don't know the input path.
    wchar_t req_string[128];
    NET_SNPRINTFW(req_string, L"/api/get-player-name/%d", req_state->player_id);

    Net_MakeHttpRequest(&NET_PLAYER_NAME_API_DESC, req_string, req_state);

    return 1;
}

bool Net_FormatPlayerNameResponse(void* input, int input_size, void* dest, NetAPIRequest* request)
{
    NetPlayerNameAPIResponse* resp_data = (NetPlayerNameAPIResponse*)dest;

    json_value_t* json_root = json_parse(input, input_size);

    if (json_root == NULL)
    {
        return false;
    }

    // TODO Don't know the response format.

    free(json_root);

    return true;
}

void Net_HandlePlayerNameResponse(NetAPIResponse* response)
{
    NetPlayerNameAPIResponse* resp_data = (NetPlayerNameAPIResponse*)response->data;
    NetPlayerNameAPIRequest* req_state = (NetPlayerNameAPIRequest*)response->request_state;

    if (response->status)
    {
        net_player_name_received->PushCell(req_state->player_id);
        net_player_name_received->PushString(resp_data->name);
        net_player_name_received->Execute();
    }

    else
    {
        net_player_name_failed->PushCell(req_state->player_id);
        net_player_name_failed->Execute();
    }
}

sp_nativeinfo_t NET_PLAYER_NAME_API_NATIVES[] = {
    sp_nativeinfo_t { "Net_RequestPlayerName", Net_RequestPlayerName },
    sp_nativeinfo_t { NULL, NULL },
};

NetAPIDesc NET_PLAYER_NAME_API_DESC = NetAPIDesc {
    sizeof(NetPlayerNameAPIResponse),
    Net_InitPlayerNameAPI,
    Net_FreePlayerNameAPI,
    Net_FormatPlayerNameResponse,
    Net_HandlePlayerNameResponse,
    NULL,
};
