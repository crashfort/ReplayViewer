#if defined NET_CALLBACKS_INCLUDED
#endinput
#endif
#define NET_CALLBACKS_INCLUDED

// Entrypoints for network callbacks.

public void Net_MapInfoDownloadReceived(int response_handle)
{
    RV_CreateZoneListMenu(response_handle);
}

public void Net_MapInfoDownloadFailed()
{
    PrintToChatAll("Could not download map information");
}

public void Net_ReplayListDownloadReceived(int response_handle)
{
    RV_ProcessDownloadedListing(response_handle);
    RV_CreateNewReplayListMenu(response_handle);
}

public void Net_ReplayListDownloadFailed()
{
    PrintToChatAll("Could not download replay list");
}

public void Net_ReplayDownloadReceived(int response_handle)
{
    RV_ProcessDownloadedReplay(response_handle);
}

public void Net_ReplayDownloadFailed()
{
    PrintToChatAll("Could not download replay");
}
