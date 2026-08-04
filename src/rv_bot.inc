#if defined BOT_INCLUDED
#endinput
#endif
#define BOT_INCLUDED

// Bot functions.

public void RV_StartMapBots()
{
    rv_playback_bot.state = RV_BOT_IDLE;

    RV_StopBot();
}

// Bots should not pick up items.
public Action RV_BlockWeaponCanUse(int client, int weapon)
{
    return Plugin_Handled;
}

public void RV_SpawnBot(char[] name, int team)
{
    rv_playback_bot.state = RV_BOT_WAITING;
    rv_playback_bot.playback_frame = 0;
}

public void RV_StopBot()
{
    RV_FreeStream(rv_playback_bot.stream);
    rv_playback_bot.state = RV_BOT_IDLE;
}

// Set new stream.
public void RV_SetBotStream(RVStream stream)
{
    RV_FreeStream(rv_playback_bot.stream);
    RV_DupeStream(stream, rv_playback_bot.stream); // Transfer.
}

public void RV_BotFinished()
{
    rv_playback_bot.state = RV_BOT_FINISHED;
    Rec_MarkState(REC_STATE_FINISHED);
}

public void RV_UpdateBotWaiting(int& buttons, float wish_vel[3], float wish_angles[3])
{
    RVFrame frame;
    GetArrayArray(rv_playback_bot.stream.frames, 0, frame);

    wish_vel = RV_ZERO_VECTOR3;

    float view_angles[3];
    RV_CopyAngles(frame.angles, view_angles);
    RV_CopyAngles(frame.angles, wish_angles);

    buttons = frame.buttons;

    TeleportEntity(rv_playback_bot.id, frame.position, view_angles, RV_ZERO_VECTOR3);
    Rec_ProvideData(frame.velocity, frame.buttons);
}

public void RV_MoveBotToStart()
{
    RVFrame frame;
    GetArrayArray(rv_playback_bot.stream.frames, 0, frame);

    float view_angles[3];
    RV_CopyAngles(frame.angles, view_angles);

    TeleportEntity(rv_playback_bot.id, frame.position, view_angles, RV_ZERO_VECTOR3);
    Rec_ProvideData(RV_ZERO_VECTOR3, frame.buttons);
}

public void RV_UpdateBotPlaying(int& buttons, float wish_vel[3], float wish_angles[3])
{
    // PrintToServer("Updating %d", rv_playback_bot.id);

    // Negative frames are used for synchronization.
    int read_frame = rv_playback_bot.playback_frame;

    if (read_frame < 0)
    {
        read_frame = 0;
    }

    RVFrame frame;
    GetArrayArray(rv_playback_bot.stream.frames, read_frame, frame);

    // Parameter overrides to ensure a smooth playback

    wish_vel = RV_ZERO_VECTOR3;

    float view_angles[3];
    RV_CopyAngles(frame.angles, view_angles);
    RV_CopyAngles(frame.angles, wish_angles);

    buttons = frame.buttons;

    // First frame just move to the start.

    if (read_frame == 0)
    {
        TeleportEntity(rv_playback_bot.id, frame.position, view_angles, RV_ZERO_VECTOR3);
        Rec_ProvideData(RV_ZERO_VECTOR3, frame.buttons);
    }

    else
    {
        float cur_pos[3];
        GetClientAbsOrigin(rv_playback_bot.id, cur_pos);

        float distance = GetVectorDistance(frame.position, cur_pos, true);

        // Teleported somewhere else.
        if (distance > 9216.0)
        {
            TeleportEntity(rv_playback_bot.id, frame.position, view_angles, NULL_VECTOR);
            Rec_ProvideData(RV_ZERO_VECTOR3, frame.buttons);
        }

        // Normal processing with just adjusting velocity between the recorded points.
        else
        {
            float new_vel[3];
            MakeVectorFromPoints(cur_pos, frame.position, new_vel);
            ScaleVector(new_vel, 1.0 / GetTickInterval());

            TeleportEntity(rv_playback_bot.id, NULL_VECTOR, view_angles, new_vel);
            Rec_ProvideData(frame.velocity, frame.buttons);
        }
    }

    rv_playback_bot.playback_frame++;

    if (rv_playback_bot.playback_frame >= rv_playback_bot.stream.num_frames)
    {
        RV_BotFinished();
    }
}

public Action RV_RunBotCmd(int& buttons, float wish_vel[3], float wish_angles[3])
{
    // PrintToServer("state = %d", rv_playback_bot.state);

    switch (rv_playback_bot.state)
    {
        case RV_BOT_WAITING:
        {
            RV_UpdateBotWaiting(buttons, wish_vel, wish_angles);
        }

        case RV_BOT_PLAYING:
        {
            RV_UpdateBotPlaying(buttons, wish_vel, wish_angles);
        }

        case RV_BOT_FINISHED:
        {
            // When finished, stay at the last frame indefinitely.
            return Plugin_Handled;
        }
    }

    return Plugin_Changed;
}

public bool RV_SpawnBotWithStream(RVStream stream, char[] name)
{
    // PrintToChatAll("A");

    RV_SpawnBot(name, stream.team);
    RV_SetBotStream(stream); // Transfer stream.
    RV_FreeStream(stream);

    return true;
}

public void RV_StartBot()
{
    rv_playback_bot.state = RV_BOT_PLAYING;
}

// Start playback.
public void RV_StartPlaybackBots()
{
    // PrintToChatAll("B");

    RV_StartBot();
    RV_MoveBotToStart();
}

// Clear all the bots and stop playback.
// New replays can be loaded after this.
public void RV_StopPlaybackBots()
{
    RV_StopBot();
}

public void RV_PauseBot()
{
    rv_playback_bot.state = RV_BOT_WAITING;
}

public void RV_ResetPlaybackBots()
{
    RV_PauseBot();
    RV_MoveBotToStart();
}
