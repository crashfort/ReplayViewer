#if defined BOT_INCLUDED
#endinput
#endif
#define BOT_INCLUDED

// Bot functions.

public Action RV_FindBots(Handle timer)
{
    // Note that we read from the server max player count here, which can change.
    // Since we are a listen server, start from index 1 (since the player is 0).
    for (int i = 1; i <= MaxClients; i++)
    {
        if (IsClientInGame(i) && IsFakeClient(i) && !IsClientSourceTV(i))
        {
            RV_InitBot(rv_bots[rv_num_bots], i);
            rv_bot_maps[i] = rv_num_bots;
            rv_num_bots++;
        }
    }

    return Plugin_Stop;
}

public void RV_StartMapBots()
{
    rv_num_bots = 0;

    for (int i = 0; i < MAXPLAYERS + 1; i++)
    {
        rv_bot_maps[i] = 0;
    }

    // Must wait a bit before we can find the bots.
    CreateTimer(1.0, RV_FindBots);
}

public int RV_GetBotIdxFromId(int id)
{
    int mapped = rv_bot_maps[id];

    if (id == 0)
    {
        return -1;
    }

    return mapped;
}

public void RV_InitBot(RVBot bot, int id)
{
    bot.id = id;
    bot.state = RV_BOT_IDLE;

    // PrintToServer("init bot id %d %d", id, bot.id);

    RV_StopBot(bot);
}

// Bots should not pick up items.
public Action RV_BlockWeaponCanUse(int client, int weapon)
{
    return Plugin_Handled;
}

public void RV_SpawnBot(RVBot bot, char[] name, int team)
{
    SetEntityFlags(bot.id, GetEntityFlags(bot.id) | FL_DONTTOUCH); // Do not touch triggers.
    SDKHook(bot.id, SDKHook_WeaponCanUse, RV_BlockWeaponCanUse); // Do not pick up items.

    ChangeClientTeam(bot.id, team);
    CS_RespawnPlayer(bot.id);

    SetEntityRenderMode(bot.id, RENDER_TRANSADD);
    SetEntityRenderColor(bot.id, 255, 255, 255, 100);

    // Having a bot in noclip and zero gravity ensures it's smooth.
    SetEntityMoveType(bot.id, MOVETYPE_NOCLIP);
    SetEntityGravity(bot.id, 0.0);

    SetClientName(bot.id, name);

    bot.state = RV_BOT_WAITING;
}

public void RV_StopBot(RVBot bot)
{
    ChangeClientTeam(bot.id, CS_TEAM_SPECTATOR);

    SetClientName(bot.id, "crashfort/ReplayViewer");

    bot.state = RV_BOT_IDLE;
}

public bool RV_IsBotPlaying(RVBot bot)
{
    return bot.state != RV_BOT_IDLE;
}

// Set new stream.
public void RV_SetBotStream(RVBot bot, RVStream stream)
{
    RV_FreeStream(bot.stream);
    RV_DupeStream(stream, bot.stream); // Transfer.
}

public void RV_FreeBot(RVBot bot)
{
    RV_FreeStream(bot.stream);
}

public void RV_BotFinished(RVBot bot)
{
    RV_StopBot(bot);
}

public void RV_UpdateBotWaiting(RVBot bot, int& buttons, float wish_vel[3], float wish_angles[3])
{
    buttons = 0;
    wish_vel = RV_ZERO_VECTOR3;
}

public void RV_MoveBotToStart(RVBot bot)
{
    RVFrame frame;
    GetArrayArray(bot.stream.frames, 0, frame);

    float view_angles[3];
    RV_CopyToAngles(frame.angles, view_angles);

    TeleportEntity(bot.id, frame.position, view_angles, RV_ZERO_VECTOR3);
}

public void RV_UpdateBotPlaying(RVBot bot, int& buttons, float wish_vel[3], float wish_angles[3])
{
    // PrintToServer("Updating %d", bot.id);

    // Negative frames are used for synchronization.
    int read_frame = bot.playback_frame;

    if (read_frame < 0)
    {
        read_frame = 0;
    }

    RVFrame frame;
    GetArrayArray(bot.stream.frames, read_frame, frame);

    // Parameter overrides to ensure a smooth playback

    wish_vel[0] = 0.0;
    wish_vel[1] = 0.0;
    wish_vel[2] = 0.0;

    float view_angles[3];
    RV_CopyToAngles(frame.angles, view_angles);

    RV_CopyToAngles(frame.angles, wish_angles);

    buttons = frame.buttons;

    // First frame just move to the start.

    if (read_frame == 0)
    {
        TeleportEntity(bot.id, frame.position, view_angles, RV_ZERO_VECTOR3);
    }

    else
    {
        float cur_pos[3];
        GetClientAbsOrigin(bot.id, cur_pos);

        float distance = GetVectorDistance(frame.position, cur_pos, true);

        // Teleported somewhere else.
        if (distance > 9216.0)
        {
            TeleportEntity(bot.id, frame.position, view_angles, NULL_VECTOR);
        }

        // Normal processing with just adjusting velocity between the recorded points.
        else
        {
            float new_vel[3];
            MakeVectorFromPoints(cur_pos, frame.position, new_vel);
            ScaleVector(new_vel, 1.0 / GetTickInterval());

            TeleportEntity(bot.id, NULL_VECTOR, view_angles, new_vel);
        }
    }

    bot.playback_frame++;

    if (bot.playback_frame >= bot.stream.num_frames)
    {
        RV_BotFinished(bot);
    }
}

public Action RV_RunBotCmd(RVBot bot, int& buttons, float wish_vel[3], float wish_angles[3])
{
    // PrintToServer("%d", bot.state);

    switch (bot.state)
    {
        case RV_BOT_WAITING:
        {
            RV_UpdateBotWaiting(bot, buttons, wish_vel, wish_angles);
        }

        case RV_BOT_PLAYING:
        {
            RV_UpdateBotPlaying(bot, buttons, wish_vel, wish_angles);
        }
    }

    return Plugin_Changed;
}

public int RV_GetFreeBotIdx()
{
    for (int i = 0; i < rv_num_bots; i++)
    {
        if (!RV_IsBotPlaying(rv_bots[i]))
        {
            return i;
        }
    }

    return -1;
}

public bool RV_CanSpawnMoreBots()
{
    return RV_GetFreeBotIdx() != -1;
}

public bool RV_SpawnBotWithStream(char[] path)
{
    int idx = RV_GetFreeBotIdx();

    if (idx == -1)
    {
        PrintToChatAll("No more free bots");
        return false;
    }

    RVStream stream;

    if (!RV_LoadFrameStream(path, stream))
    {
        return false;
    }

#if 1
    char path_copy[PLATFORM_MAX_PATH];
    strcopy(path_copy, sizeof(path_copy), path);

    ReplaceString(path_copy, sizeof(path_copy), ".rec", "");

    int name_idx = FindCharInString(path_copy, '/', true) + 1;

    char bot_name[128];
    strcopy(bot_name, sizeof(bot_name), path_copy[name_idx]);
#endif

#if 0
    char bot_name[128];
    char time_buf[64];

    RV_FormatTime(time_buf, sizeof(time_buf), RV_FramesToTime(stream.duration, GetTickInterval()));

    RVFileMeta meta;

    if (RV_ParseFileMeta(path, meta))
    {
        Format(bot_name, sizeof(bot_name), "[%s] %d", time_buf, meta.player_id);
    }

    else
    {
        Format(bot_name, sizeof(bot_name), "[%s]", time_buf);
    }
#endif

    RV_SpawnBot(rv_bots[idx], bot_name, stream.team);
    RV_SetBotStream(rv_bots[idx], stream); // Transfer stream.

    RV_FreeStream(stream);

    return true;
}

public void RV_StartBot(RVBot bot)
{
    bot.state = RV_BOT_PLAYING;
}

// Start playback.
public void RV_StartPlaybackBots()
{
    for (int i = 0; i < rv_num_bots; i++)
    {
        if (RV_IsBotPlaying(rv_bots[i]))
        {
            RV_StartBot(rv_bots[i]);
            RV_MoveBotToStart(rv_bots[i]);
        }
    }

    // Offset everyone to the slowest starter.
    int slowest_starter = 0;

    for (int i = 0; i < rv_num_bots; i++)
    {
        if (RV_IsBotPlaying(rv_bots[i]))
        {
            if (rv_bots[i].stream.start_offset > slowest_starter)
            {
                slowest_starter = rv_bots[i].stream.start_offset;
            }
        }
    }

    for (int i = 0; i < rv_num_bots; i++)
    {
        if (RV_IsBotPlaying(rv_bots[i]))
        {
            int offset = slowest_starter - rv_bots[i].stream.start_offset;
            rv_bots[i].playback_frame = -offset;
        }
    }
}

// Clear all the bots and stop playback.
// New replays can be loaded after this.
public void RV_StopPlaybackBots()
{
    for (int i = 0; i < rv_num_bots; i++)
    {
        RV_StopBot(rv_bots[i]);
    }
}

public void RV_PauseBot(RVBot bot)
{
    bot.state = RV_BOT_WAITING;
}

public void RV_ResetPlaybackBots()
{
    for (int i = 0; i < rv_num_bots; i++)
    {
        if (RV_IsBotPlaying(rv_bots[i]))
        {
            RV_PauseBot(rv_bots[i]);
            RV_MoveBotToStart(rv_bots[i]);
        }
    }
}
