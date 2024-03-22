#if defined ROUND_INCLUDED
#endinput
#endif
#define ROUND_INCLUDED

// Don't allow the round to end.
public Action CS_OnTerminateRound(float& delay, CSRoundEndReason& reason)
{
    return Plugin_Handled;
}

public void RV_InitRound()
{
}
