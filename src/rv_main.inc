#if defined MAIN_INCLUDED
#endinput
#endif
#define MAIN_INCLUDED

// Events from SourceMod.

public void RV_CreateDirs()
{
    char replays_path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, replays_path, sizeof(replays_path), "%s/%s", RV_ROOT_PATH, "replays");

    RV_MaybeCreateDir(RV_ROOT_PATH);
    RV_MaybeCreateDir(replays_path);
}

public void RV_PluginStart()
{
    RV_InitEngine();
    RV_InitPlayer();

    RV_CreateDirs();
    RV_RegisterCmds();
}

public void RV_PluginEnd()
{
}

public void RV_MapStart()
{
    GetCurrentMap(RV_CurrentMap, sizeof(RV_CurrentMap));

    RV_SystemState = RV_SYS_IDLE;

    RV_StartMapCmds();
    RV_StartMapBots();
}

public void RV_MapEnd()
{
}

public Action RV_RunPlayerCmd(int client, int& buttons, float wishvel[3], float wishangles[3])
{
    // Bots should not update during pause.
    if (RV_SystemState == RV_SYS_PLAYING)
    {
        if (IsFakeClient(client))
        {
            if (GetClientTeam(client) != CS_TEAM_SPECTATOR)
            {
                int idx = RV_GetBotIdxFromId(client);

                if (idx != -1)
                {
                    return RV_RunBotCmd(RV_Bots[idx], buttons, wishvel, wishangles);
                }
            }
        }
    }

    return Plugin_Continue;
}

public void RV_StartPlayback()
{
    RV_SystemState = RV_SYS_PLAYING;
    RV_StartPlaybackBots();
}

public void RV_StopPlayback()
{
    RV_SystemState = RV_SYS_IDLE;
    RV_StopPlaybackBots();
}

public void RV_ResetPlayback()
{
    RV_SystemState = RV_SYS_PAUSED;
}
