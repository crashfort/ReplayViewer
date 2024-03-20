#if defined BOT_INCLUDED
#endinput
#endif
#define BOT_INCLUDED

// Bot functions.

public Action RV_FindBots(Handle timer)
{
    int index = 0;

    // Note that we read from the server max player count here, which can change.
    for (int i = 1; i < MaxClients; i++)
    {
        if (IsClientInGame(i) && IsFakeClient(i) && !IsClientSourceTV(i))
        {
            RV_InitBot(RV_Bots[index], i);
            index++;
        }
    }

    return Plugin_Stop;
}

public void RV_StartBots()
{
    // Must wait a bit before we can find the bots.
    CreateTimer(1.0, RV_FindBots);
}

public void RV_InitBot(RVBot bot, int id)
{
    bot.id = id;
    
    ChangeClientTeam(bot.id, CS_TEAM_SPECTATOR);

    char name_buf[MAX_NAME_LENGTH];
    FormatEx(name_buf, sizeof(name_buf), "crashfort/ReplayViewer");

    SetClientName(bot.id, name_buf);
}

public void RV_SpawnBot(RVBot bot)
{
    CS_RespawnPlayer(bot.id);

    SetEntityRenderMode(bot.id, RENDER_TRANSADD);
    SetEntityRenderColor(bot.id, 255, 255, 255, 100);

    // Having a bot in noclip and zero gravity ensures it's smooth
    SetEntityMoveType(bot.id, MOVETYPE_NOCLIP);
    SetEntityGravity(bot.id, 0.0);
}

public bool RV_IsBotPlaying(RVBot bot)
{
    int team = GetClientTeam(bot.id);
    return team == CS_TEAM_T || team == CS_TEAM_CT;
}

public Action RV_RunBotCommand(int client, int& buttons, float wishvel[3], float wishangles[3])
{
    return Plugin_Continue;
}
