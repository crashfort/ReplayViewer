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
            RV_InitBot(RV_Bots[i], i);
            RV_BotsUsed[i] = true;
        }
    }

    return Plugin_Stop;
}

public void RV_StartMapBots()
{
    // Must wait a bit before we can find the bots.
    CreateTimer(1.0, RV_FindBots);
}

public void RV_InitBot(RVBot bot, int id)
{
    bot.id = id;
    bot.frames = null;

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

// Set new array of RVFrame.
public void RV_SetBotFrames(RVBot bot, ArrayList frames)
{
    if (bot.frames != null)
    {
        delete bot.frames;
        bot.frames = null;
    }

    bot.frames = RV_CloneArrayHandle(frames); // Transfer.
    bot.playback_tick = 0;
    bot.playback_length = GetArraySize(bot.frames);
}

public void RV_FreeBot(RVBot bot)
{
    if (bot.frames != null)
    {
        delete bot.frames;
        bot.frames = null;
    }
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

    if (bot.frames == null)
    {
        // PrintToServer("No frames %d", bot.id);
        return Plugin_Handled;
    }

    // Parameter overrides to ensure a smooth playback
    wish_vel[0] = 0.0;
    wish_vel[1] = 0.0;
    wish_vel[2] = 0.0;

    // PrintToServer("Updating %d", bot.id);

    RVFrame frame;
    GetArrayArray(bot.frames, bot.playback_tick, frame);

    float view_angles[3];
    view_angles[0] = frame.angles[0];
    view_angles[1] = frame.angles[1];
    view_angles[2] = 0.0;

    wish_angles[0] = frame.angles[0];
    wish_angles[1] = frame.angles[1];

    // First tick just move to the start.

    if (bot.playback_tick == 0)
    {
        TeleportEntity(bot.id, frame.position, view_angles, NULL_VECTOR);
    }

    else
    {
        float cur_pos[3];
        GetClientAbsOrigin(bot.id, cur_pos);

        float distance = GetVectorDistance(frame.position, cur_pos, true);

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

    bot.playback_tick++;

    if (bot.playback_tick >= bot.playback_length)
    {
        bot.playback_tick = 0;
    }

    return Plugin_Changed;
}

public int RV_GetFreeBotIdx()
{
    // Since we are a listen server, start from index 1 (since the player is 0).
    for (int i = 1; i <= MaxClients; i++)
    {
        if (RV_BotsUsed[i])
        {
            if (!RV_IsBotPlaying(RV_Bots[i]))
            {
                return i;
            }
        }
    }

    return -1;
}

public bool RV_SpawnBotWithStream(char[] path)
{
    int free_idx = RV_GetFreeBotIdx();

    if (free_idx == -1)
    {
        PrintToChatAll("No more free bots");
        return false;
    }

    ArrayList frames = RV_LoadFrameStream(path);

    if (frames == null)
    {
        return false;
    }

    // TODO Need to get what team to start on from the stream.
    RV_SpawnBot(RV_Bots[free_idx], "Playing", CS_TEAM_T);
    RV_SetBotFrames(RV_Bots[free_idx], frames); // Transfer stream.

    delete frames;

    return true;
}

public void RV_ResetBot(RVBot bot)
{
    bot.playback_tick = 0;
}

// Start playback.
public void RV_StartPlaybackBots()
{
    for (int i = 1; i <= MaxClients; i++)
    {
        if (RV_BotsUsed[i])
        {
            RV_ResetBot(RV_Bots[i]);
        }
    }
}

// Clear all the bots and stop playback.
// New replays can be loaded after this.
public void RV_StopPlaybackBots()
{
    for (int i = 1; i <= MaxClients; i++)
    {
        if (RV_BotsUsed[i])
        {
            RV_ResetBot(RV_Bots[i]);
            RV_StopBot(RV_Bots[i]);
        }
    }
}
