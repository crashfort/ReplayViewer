#if defined PLAYER_INCLUDED
#endinput
#endif
#define PLAYER_INCLUDED

// Stuff for all players (including bots and the main player).

public Action RV_OnPlayerSpawn(Event event, const char[] name, bool dont_broadcast)
{
    int userid = GetEventInt(event, "userid");
    int client = GetClientOfUserId(userid);

    // Disable player to player collision.
    Eng_SetClientCollisionGroup(client, ENG_COLLISION_GROUP_DEBRIS_TRIGGER);

    return Plugin_Continue;
}

public void RV_InitPlayer()
{
    // Catches the local player and also all bots.
    HookEvent("player_spawn", RV_OnPlayerSpawn);
}
