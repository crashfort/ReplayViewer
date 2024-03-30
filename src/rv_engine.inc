#if defined ENGINE_INCLUDED
#endinput
#endif
#define ENGINE_INCLUDED

// External stuff that is not defined by SourceMod.

// Constants for Eng_SetClientCollisionGroup.
#define ENG_COLLISION_GROUP_DEBRIS_TRIGGER 2

// Constants for Eng_SetClientObserverMode.
#define ENG_OBS_MODE_NONE 0
#define ENG_OBS_MODE_DEATHCAM 1
#define ENG_OBS_MODE_FREEZECAM 2
#define ENG_OBS_MODE_FIXED 3
#define ENG_OBS_MODE_IN_EYE 4
#define ENG_OBS_MODE_CHASE 5
#define ENG_OBS_MODE_ROAMING 6

public void Eng_SetClientCollisionGroup(int client, int group)
{
    SetEntProp(client, Prop_Send, "m_CollisionGroup", group);
}

public void Eng_SetClientObserverMode(int client, int mode)
{
    SetEntProp(client, Prop_Send, "m_iObserverMode", mode);
}

// Used by spectate modes that follow a player.
public void Eng_SetClientObserverTarget(int client, int target)
{
    SetEntPropEnt(client, Prop_Send, "m_hObserverTarget", target);
}
