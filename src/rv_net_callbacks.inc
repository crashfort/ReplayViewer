#if defined NET_CALLBACKS_INCLUDED
#endinput
#endif
#define NET_CALLBACKS_INCLUDED

// Entrypoints for network callbacks.

public void Net_MapInfoDownloadReceived()
{
    RV_CreateZoneListMenu();
}

public void Net_MapInfoDownloadFailed()
{
    PrintToChatAll("Could not download map information");
}

public void Net_ReplayListDownloadReceived()
{
    RV_ProcessDownloadedListing();
    RV_CreateNewReplayListMenu();
}

public void Net_ReplayListDownloadFailed()
{
    PrintToChatAll("Could not download replay list");
}

public void Net_ReplayDownloadReceived()
{
    RV_ProcessDownloadedReplay();
}

public void Net_ReplayDownloadFailed()
{
    PrintToChatAll("Could not download replay");
}
