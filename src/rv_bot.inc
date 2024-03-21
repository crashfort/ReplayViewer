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
            RV_InitBot(RV_Bots[RV_NumBots], i);
            RV_BotMaps[i] = RV_NumBots;
            RV_NumBots++;
        }
    }

    return Plugin_Stop;
}

public void RV_StartMapBots()
{
    RV_NumBots = 0;

    for (int i = 0; i < MAXPLAYERS + 1; i++)
    {
        RV_BotMaps[i] = 0;
    }

    // Must wait a bit before we can find the bots.
    CreateTimer(1.0, RV_FindBots);
}

public int RV_GetBotIdxFromId(int id)
{
    int mapped = RV_BotMaps[id];

    if (id == 0)
    {
        return -1;
    }

    return mapped;
}

public void RV_InitBot(RVBot bot, int id)
{
    bot.id = id;

    // PrintToServer("init bot id %d %d", id, bot.id);

    RV_StopBot(bot);
}

public void RV_SpawnBot(RVBot bot, char[] name, int team)
{
    ChangeClientTeam(bot.id, team);
    CS_RespawnPlayer(bot.id);

    SetEntityRenderMode(bot.id, RENDER_TRANSADD);
    SetEntityRenderColor(bot.id, 255, 255, 255, 100);

    // Having a bot in noclip and zero gravity ensures it's smooth.
    SetEntityMoveType(bot.id, MOVETYPE_NOCLIP);
    SetEntityGravity(bot.id, 0.0);

    SetClientName(bot.id, name);
}

public void RV_StopBot(RVBot bot)
{
    ChangeClientTeam(bot.id, CS_TEAM_SPECTATOR);

    char name_buf[MAX_NAME_LENGTH];
    FormatEx(name_buf, sizeof(name_buf), "crashfort/ReplayViewer");

    SetClientName(bot.id, name_buf);
}

public bool RV_IsBotPlaying(RVBot bot)
{
    int team = GetClientTeam(bot.id);
    return team == CS_TEAM_T || team == CS_TEAM_CT;
}

// Set new stream.
public void RV_SetBotStream(RVBot bot, RVStream stream)
{
    RV_FreeStream(bot.stream);
    RV_DupeStream(stream, bot.stream); // Transfer.

    RV_ResetBot(bot);
}

public void RV_FreeBot(RVBot bot)
{
    RV_FreeStream(bot.stream);
}

public void RV_BotFinished(RVBot bot)
{
    RV_ResetBot(bot);
    RV_StopBot(bot);
}

public Action RV_RunBotCmd(RVBot bot, int& buttons, float wish_vel[3], float wish_angles[3])
{
    if (!RV_PlaybackActive)
    {
        // Bots should not update during pause.
        return Plugin_Handled;
    }

    if (!RV_IsBotPlaying(bot))
    {
        return Plugin_Handled;
    }

    if (bot.stream.frames == null)
    {
        return Plugin_Handled;
    }

    // Parameter overrides to ensure a smooth playback
    wish_vel[0] = 0.0;
    wish_vel[1] = 0.0;
    wish_vel[2] = 0.0;

    // PrintToServer("Updating %d", bot.id);

    // Negative frames are used for synchronization.
    int read_frame = bot.playback_frame;

    if (read_frame < 0)
    {
        read_frame = 0;
    }

    RVFrame frame;
    GetArrayArray(bot.stream.frames, read_frame, frame);

    float view_angles[3];
    view_angles[0] = frame.angles[0];
    view_angles[1] = frame.angles[1];
    view_angles[2] = 0.0;

    wish_angles[0] = frame.angles[0];
    wish_angles[1] = frame.angles[1];

    // First frame just move to the start.

    if (bot.playback_frame <= 0)
    {
        TeleportEntity(bot.id, frame.position, view_angles, NULL_VECTOR);
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

    return Plugin_Changed;
}

public int RV_GetFreeBotIdx()
{
    for (int i = 0; i < RV_NumBots; i++)
    {
        if (!RV_IsBotPlaying(RV_Bots[i]))
        {
            return i;
        }
    }

    return -1;
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

    // TODO Need to get what team to start on from the stream.
    RV_SpawnBot(RV_Bots[idx], "Playing", CS_TEAM_T);
    RV_SetBotStream(RV_Bots[idx], stream); // Transfer stream.

    RV_FreeStream(stream);

    return true;
}

public void RV_ResetBot(RVBot bot)
{
    bot.playback_frame = 0;
}

// Start playback.
public void RV_StartPlaybackBots()
{
    for (int i = 0; i < RV_NumBots; i++)
    {
        RV_ResetBot(RV_Bots[i]);
    }

    // Offset everyone to the slowest starter.
    int slowest_starter = 0;

    for (int i = 0; i < RV_NumBots; i++)
    {
        if (RV_IsBotPlaying(RV_Bots[i]))
        {
            if (RV_Bots[i].stream.start_offset > slowest_starter)
            {
                slowest_starter = RV_Bots[i].stream.start_offset;
            }
        }
    }

    for (int i = 0; i < RV_NumBots; i++)
    {
        if (RV_IsBotPlaying(RV_Bots[i]))
        {
            int offset = slowest_starter - RV_Bots[i].stream.start_offset;
            RV_Bots[i].playback_frame = -offset;
        }
    }
}

// Clear all the bots and stop playback.
// New replays can be loaded after this.
public void RV_StopPlaybackBots()
{
    for (int i = 0; i < RV_NumBots; i++)
    {
        RV_ResetBot(RV_Bots[i]);
        RV_StopBot(RV_Bots[i]);
    }
}
