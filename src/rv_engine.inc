#if defined ENGINE_INCLUDED
#endinput
#endif
#define ENGINE_INCLUDED

// External stuff that is not defined by SourceMod.

#define COLLISION_GROUP_DEBRIS_TRIGGER 2

public int RV_OffsetCollisionGroup;

public void RV_InitEngine()
{
    RV_OffsetCollisionGroup = FindSendPropInfo("CBaseEntity", "m_CollisionGroup");
}
