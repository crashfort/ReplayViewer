#include <sourcemod>
#include <sdktools>
#include <cstrike>
#include <string>
#include <keyvalues>
#include <sdkhooks>
#include <sdktools>
#include <menus>

#pragma semicolon 1
#pragma newdecls required

#include "rv_engine.inc"
#include "rv_util.inc"
#include "rv_net.inc"
#include "rv_priv.inc"
#include "rv_cmds.inc"
#include "rv_menu.inc"
#include "rv_main.inc"
#include "rv_bot.inc"
#include "rv_load.inc"
#include "rv_player.inc"
#include "rv_round.inc"
#include "rv_net_callbacks.inc"
#include "rv_nav.inc"
#include "rv_delays.inc"

// Main interface from SourceMod.

public Plugin my_info =
{
    name = "Replay viewer",
    author = "crashfort",
    description = "",
    version = "1",
    url = "https://github.com/crashfort/ReplayViewer"
};

public void OnPluginStart()
{
    RV_PluginStart();
}

public void OnPluginEnd()
{
    RV_PluginEnd();
}

public void OnMapStart()
{
    RV_MapStart();
}

public void OnMapEnd()
{
    RV_MapEnd();
}

public Action OnPlayerRunCmd(int client, int& buttons, int& impulse, float wishvel[3], float wishangles[3])
{
    return RV_RunPlayerCmd(client, buttons, wishvel, wishangles);
}
