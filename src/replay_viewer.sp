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

#include "rv_priv.inc"
#include "rv_cmds.inc"
#include "rv_menu.inc"
#include "rv_main.inc"
#include "rv_bot.inc"

// Main interface from SourceMod.

public Plugin my_info =
{
    name = "Replay viewer",
    author = "",
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
    if (IsFakeClient(client))
    {
        return RV_RunBotCommand(client, buttons, wishvel, wishangles);
    }

    return Plugin_Continue;
}
