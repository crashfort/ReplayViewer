#if defined CMDS_INCLUDED
#endinput
#endif
#define CMDS_INCLUDED

// Console commands.

// Main user interaction with the system.
public Action RV_OpenInterfaceCmd(int client, int args)
{
    RV_OpenInterfaceMenu(client);
    return Plugin_Handled;
}

public void RV_RegisterCmds()
{
    RegConsoleCmd("sm_replay_viewer", RV_OpenInterfaceCmd);
}

// Set some prerequisites that we want.
public void RV_StartCmds()
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

    // Don't let the bot commands be overwritten.
    ServerExecute();
}
