#if defined CMDS_INCLUDED
#endinput
#endif
#define CMDS_INCLUDED

// Console commands.

// Just used to set which file to load.
public Action RV_LoadReplayCmd(int client, int args)
{
    GetCmdArg(1, rv_pending_replay, sizeof(rv_pending_replay));

    if (strlen(rv_pending_replay) > 0)
    {
        if (RV_LoadLocalFile(rv_pending_replay))
        {
            Rec_MarkState(REC_STATE_LOADED);
        }

        else
        {
            Rec_MarkState(REC_STATE_ERROR);
        }
    }

    return Plugin_Handled;
}

public Action RV_StartReplayCmd(int client, int args)
{
    RV_StartPlayback();
    Rec_MarkState(REC_STATE_PLAYING);
    return Plugin_Handled;
}

public void RV_RegisterCmds()
{
    RegConsoleCmd("sm_load_replay", RV_LoadReplayCmd);
    RegConsoleCmd("sm_start_replay", RV_StartReplayCmd);
}

// Set some prerequisites that we want.
public void RV_StartMapCmds()
{
    ServerCommand("sv_cheats 1");
    ServerExecute();
}
