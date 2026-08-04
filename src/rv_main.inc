#if defined MAIN_INCLUDED
#endinput
#endif
#define MAIN_INCLUDED

// Events from SourceMod.

public void RV_CreateDirs()
{
    char replays_path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, replays_path, sizeof(replays_path), "%s/replays", RV_ROOT_PATH);

    RV_MaybeCreateDir(RV_ROOT_PATH);
    RV_MaybeCreateDir(replays_path);
}

public void RV_PluginStart()
{
    RV_InitPlayer();
    RV_InitRound();

    RV_CreateDirs();
    RV_RegisterCmds();
}

public void RV_PluginEnd()
{
}

public void RV_MapStart()
{
    GetCurrentMap(rv_cur_map, sizeof(rv_cur_map));

    RV_StartNav();
    RV_StartDelays();
    RV_StartMapCmds();
    RV_StartMapBots();
    RV_StartLoad();
}

public void RV_MapEnd()
{
    RV_StopLoad();
}

public Action RV_RunPlayerCmd(int client, int& buttons, float wishvel[3], float wishangles[3])
{
    if (IsFakeClient(client))
    {
        if (GetClientTeam(client) != CS_TEAM_SPECTATOR)
        {
            return RV_RunBotCmd(buttons, wishvel, wishangles);
        }
    }

    return Plugin_Continue;
}

public void RV_StartPlayback()
{
    RV_StartPlaybackBots();
}

public void RV_StopPlayback()
{
    RV_StopPlaybackBots();
}

public void RV_ResetPlayback()
{
    RV_ResetPlaybackBots();
}
