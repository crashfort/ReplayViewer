#if defined DELAYS_INCLUDED
#endinput
#endif
#define DELAYS_INCLUDED

// Stuff that using delays from the map start to keep things ordered.

public void RV_StartDelays()
{
    CreateTimer(1.0, RV_SpawnBots); // Must wait a bit before we can spawn stuff when creating dummy nav.
    CreateTimer(1.0, RV_RemoveJails); // Must wait a bit before we can remove stuff.
    CreateTimer(2.0, RV_FindBots); // Must wait a bit before we can find the bots.
}
