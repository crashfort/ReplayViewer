#if defined DELAYS_INCLUDED
#endinput
#endif
#define DELAYS_INCLUDED

// Stuff that using delays from the map start to keep things ordered.

// Remove entities that teleport everyone to the jail after a timeout.
public Action RV_RemoveJails(Handle timer)
{
    ServerCommand("ent_remove_all logic_timer");
    ServerExecute();
    return Plugin_Stop;
}

public Action RV_SpawnBots(Handle timer)
{
    ServerCommand("nav_load"); // Use the dummy file we created in RV_StartNav.
    ServerExecute();

    ServerCommand("bot_chatter off");
    ServerCommand("bot_stop 1");

    ServerCommand("bot_quota 1");
    ServerCommand("bot_zombie 1");
    ServerCommand("bot_stop 1");
    ServerCommand("mp_autoteambalance 0");
    ServerCommand("bot_join_after_player 0");
    ServerCommand("mp_limitteams 0");

    // Don't let the bot commands be overwritten.
    ServerExecute();

    return Plugin_Stop;
}

public Action RV_SetupBots(Handle timer)
{
    for(int i = 1; i <= MaxClients; i++)
    {
        if (IsFakeClient(i) && !IsClientSourceTV(i))
        {
            rv_playback_bot.id = i;
            break;
        }
    }

    DispatchSpawn(rv_playback_bot.id);
    ActivateEntity(rv_playback_bot.id);
    CS_UpdateClientModel(rv_playback_bot.id);
    CS_RespawnPlayer(rv_playback_bot.id);

    SetEntityFlags(rv_playback_bot.id, GetEntityFlags(rv_playback_bot.id) | FL_DONTTOUCH); // Do not touch triggers.
    SDKHook(rv_playback_bot.id, SDKHook_WeaponCanUse, RV_BlockWeaponCanUse); // Do not pick up items.

    // Having a bot in noclip and zero gravity ensures it's smooth.
    SetEntityMoveType(rv_playback_bot.id, MOVETYPE_NOCLIP);
    SetEntityGravity(rv_playback_bot.id, 0.0);

    for (int i = 0; i < 6; i++)
    {
        int weapon = GetPlayerWeaponSlot(rv_playback_bot.id, i);

        if (weapon != -1)
        {
            RemovePlayerItem(rv_playback_bot.id, weapon);
        }
    }

    return Plugin_Stop;
}

public Action RV_NotifySyncTask(Handle timer)
{
    Rec_MarkState(REC_STATE_LOADED);
    return Plugin_Stop;
}

public void RV_StartDelays()
{
    CreateTimer(1.0, RV_RemoveJails); // Must wait a bit before we can remove stuff.
    CreateTimer(1.0, RV_SpawnBots);
    CreateTimer(2.0, RV_SetupBots);
    CreateTimer(2.5, RV_NotifySyncTask);
}
