#if defined CMDS_INCLUDED
#endinput
#endif
#define CMDS_INCLUDED

// Console commands.

public Action RV_MainMenuCmd(int client, int args)
{
    DisplayMenu(rv_main_menu, client, MENU_TIME_FOREVER);
    return Plugin_Handled;
}

public void RV_RegisterCmds()
{
    RegConsoleCmd("sm_replay_viewer", RV_MainMenuCmd);
}

public Action RV_SpawnBots(Handle timer)
{
    ServerCommand("nav_load"); // Use the dummy file we created in RV_StartNav.
    ServerExecute();

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

    return Plugin_Stop;
}

// Remove entities that teleport everyone to the jail after a timeout.
public Action RV_RemoveJails(Handle timer)
{
    ServerCommand("ent_remove_all logic_timer");
    ServerExecute();
    return Plugin_Stop;
}

// Set some prerequisites that we want.
public void RV_StartMapCmds()
{
    ServerCommand("sv_cheats 1");
    ServerExecute();
}
