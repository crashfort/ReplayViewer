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

    RV_PlaybackActive = false;
    RV_HaveBotsPlaying = false;

    RV_StartMapCmds();
    RV_StartMapBots();
}

public void RV_MapEnd()
{
}

public Action RV_RunPlayerCmd(int client, int& buttons, float wishvel[3], float wishangles[3])
{
    if (IsFakeClient(client))
    {
        if (RV_BotsUsed[client])
        {
            return RV_RunBotCmd(RV_Bots[client], buttons, wishvel, wishangles);
        }
    }

    return Plugin_Continue;
}

public void RV_StartPlayback()
{
    RV_PlaybackActive = true;
    RV_StartPlaybackBots();
}

public void RV_StopPlayback()
{
    RV_PlaybackActive = false;
    RV_HaveBotsPlaying = false;

    RV_StopPlaybackBots();
}

public void RV_ResetPlayback()
{
    RV_PlaybackActive = false;
}
