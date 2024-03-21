#if defined PLAYER_INCLUDED
#endinput
#endif
#define PLAYER_INCLUDED

// Player overrides.

public Action RV_OnPlayerSpawn(Event event, const char[] name, bool dont_broadcast)
{
    int userid = GetEventInt(event, "userid");
    int client = GetClientOfUserId(userid);

    // Disables player collision.
    SetEntData(client, RV_OffsetCollisionGroup, COLLISION_GROUP_DEBRIS_TRIGGER, 4, true);

    return Plugin_Continue;
}

public void RV_InitPlayer()
{
    HookEvent("player_spawn", RV_OnPlayerSpawn);
}
