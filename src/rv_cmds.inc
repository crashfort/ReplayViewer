#if defined CMDS_INCLUDED
#endinput
#endif
#define CMDS_INCLUDED

// Console commands.

public Action RV_OpenInterfaceCmd(int client, int args)
{
    RV_OpenInterfaceMenu(client);
    return Plugin_Handled;
}

// Test to just load and print a stream.
public Action RV_PrintStreamCmd(int client, int args)
{
    if (args == 0)
    {
        PrintToChatAll("Usage: sm_rv_print_stream <file>");
        return Plugin_Handled;
    }

    char name[128];
    GetCmdArg(1, name, sizeof(name)); // Args start at 1 apparently, but the check above is still right???

    char path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, path, sizeof(path), "%s/replays/%s", RV_ROOT_PATH, name);

    RVStream stream;

    if (!RV_LoadFileStream(path, stream))
    {
        return Plugin_Handled;
    }

    for (int i = 0; i < stream.num_frames; i++)
    {
        RVFrame frame;
        GetArrayArray(stream.frames, i, frame);

        PrintToServer("--------------- frame %d", i);
        PrintToServer("buttons=%d", frame.buttons);
        PrintToServer("pos=%0.2f %0.2f %0.2f", frame.position[0], frame.position[1], frame.position[2]);
        PrintToServer("angles=%0.2f %0.2f", frame.angles[0], frame.angles[1]);
        PrintToServer("velo=%0.2f %0.2f %0.2f", frame.velocity[0], frame.velocity[1], frame.velocity[2]);
    }

    for (int i = 0; i < stream.num_bookmarks; i++)
    {
        RVBookmark bk;
        GetArrayArray(stream.bookmarks, i, bk);

        PrintToServer("--------------- event %d", i);
        PrintToServer("frame=%d", bk.frame);
        PrintToServer("stage=%d", bk.stage);
    }

    RV_FreeStream(stream);

    return Plugin_Handled;
}

// Access state should be synchronized with rv_menu.inc.

// Alternative to using the menu interface.
public Action RV_StartPlaybackCmd(int client, int args)
{
    // Messy because this language does not have fall through switch cases.
    if (rv_sys_state == RV_SYS_REPLAYS_LOADED || rv_sys_state == RV_SYS_PAUSED)
    {
        RV_StartPlayback();
    }

    return Plugin_Handled;
}

// Alternative to using the menu interface.
public Action RV_StopPlaybackCmd(int client, int args)
{
    // Messy because this language does not have fall through switch cases.
    if (rv_sys_state == RV_SYS_REPLAYS_LOADED || rv_sys_state == RV_SYS_PLAYING || rv_sys_state == RV_SYS_PAUSED)
    {
        RV_StopPlayback();
    }

    return Plugin_Handled;
}

// Alternative to using the menu interface.
public Action RV_ResetPlaybackCmd(int client, int args)
{
    if (rv_sys_state == RV_SYS_PLAYING)
    {
        RV_ResetPlayback();
    }

    return Plugin_Handled;
}

// Alternative to using the menu interface.
public Action RV_LoadAllReplaysCmd(int client, int args)
{
    if (rv_sys_state == RV_SYS_IDLE)
    {
        RV_AddAllFileReplaysForMap();
    }

    return Plugin_Handled;
}

public void RV_ShowReplayTimes()
{
    float frame_time = GetGameFrameTime();
    char time_buf[64];
    char name_buf[64];

    // Messy because this language does not have fall through switch cases.
    if (rv_sys_state == RV_SYS_REPLAYS_LOADED || rv_sys_state == RV_SYS_PAUSED)
    {
        PrintToChatAll("Durations of replays:");

        for (int i = 0; i < rv_num_bots; i++)
        {
            if (RV_IsBotPlaying(rv_bots[i]))
            {
                GetClientName(rv_bots[i].id, name_buf, sizeof(name_buf));
                RV_FormatTime(time_buf, sizeof(time_buf), RV_FramesToTime(rv_bots[i].stream.duration, frame_time));

                PrintToChatAll("%s: %s", name_buf, time_buf);
            }
        }
    }
}

public Action RV_PrintTimesCmd(int client, int args)
{
    RV_ShowReplayTimes();
    return Plugin_Handled;
}

// TODO Disable when server have the needed routes.
public Action RV_TestDownloadCmd(int client, int args)
{
    if (args < 2)
    {
        PrintToChatAll("Usage: sm_test_download <zone id> <replay rank>");
        return Plugin_Handled;
    }

    char arg_zone_id[128];
    GetCmdArg(1, arg_zone_id, sizeof(arg_zone_id));

    char arg_rank[128];
    GetCmdArg(2, arg_rank, sizeof(arg_rank));

    int zone_id = StringToInt(arg_zone_id);
    int rank = StringToInt(arg_rank);

    Net_DownloadReplay(0, zone_id, NET_ANGLE_TYPE_FW, rank);
    return Plugin_Handled;
}

public void RV_RegisterCmds()
{
    RegConsoleCmd("sm_replay_viewer", RV_OpenInterfaceCmd);
    // RegConsoleCmd("sm_print_stream", RV_PrintStreamCmd);
    RegConsoleCmd("sm_start_playback", RV_StartPlaybackCmd);
    RegConsoleCmd("sm_stop_playback", RV_StopPlaybackCmd);
    RegConsoleCmd("sm_reset_playback", RV_ResetPlaybackCmd);
    RegConsoleCmd("sm_load_all_replays", RV_LoadAllReplaysCmd);
    RegConsoleCmd("sm_print_times", RV_PrintTimesCmd);
    RegConsoleCmd("sm_test_download", RV_TestDownloadCmd); // TODO Disable when server have the needed routes.
}

// Remove entities that teleport everyone to the jail after a timeout.
public Action RV_RemoveJails(Handle timer)
{
    ServerCommand("ent_remove_all logic_timer");
    return Plugin_Stop;
}

// Set some prerequisites that we want.
public void RV_StartMapCmds()
{
    ServerCommand("sv_cheats 1");

    ServerCommand("bot_chatter off");
    ServerCommand("bot_stop 1");

    ServerCommand("bot_quota %d", MAXPLAYERS - 1);
    ServerCommand("bot_zombie 1");
    ServerCommand("bot_stop 1");
    ServerCommand("mp_autoteambalance 0");
    ServerCommand("bot_join_after_player 0");
    ServerCommand("mp_limitteams 0");

    CreateTimer(1.0, RV_RemoveJails); // Must wait a bit before we can remove stuff.

    // Don't let the bot commands be overwritten.
    ServerExecute();
}
