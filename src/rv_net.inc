#if defined NET_INCLUDED
#endinput
#endif
#define NET_INCLUDED

// Internet functions.

// Zone 0 is the main map, 1 - 30 are stages and 31 - 40 are bonuses.
#define NET_ZONE_MAP 0
#define NET_ZONE_FIRST_STAGE 1
#define NET_ZONE_FIRST_BONUS 31

#define NET_ANGLE_TYPE_FW 0
#define NET_ANGLE_TYPE_SW 1
#define NET_ANGLE_TYPE_HSW 2
#define NET_ANGLE_TYPE_BW 3

#define NET_NULL_HANDLE 0

// Index 0 will return stage 1.
stock int Net_GetStageZone(int idx)
{
    return NET_ZONE_FIRST_STAGE + idx;
}

// Index 0 will return bonus 1.
stock int Net_GetBonusZone(int idx)
{
    return NET_ZONE_FIRST_BONUS + idx;
}

// Returns true if a connection to the Internet is available right now.
// Calls to other net functions can not be done if this returns false.
native bool Net_ConnectedToInet();

// Call this to free a response.
// A response handle is given on success through the forwards below.
native void Net_CloseHandle(int response_handle);

// Get information about the current map, such as the zones.
// The user id is optional, but can be used to identify parallel requests.
native bool Net_DownloadMapInfo(int user_id);
forward void Net_MapInfoDownloadReceived(int user_id, int response_handle); // Sent in response to Net_DownloadMapInfo to return the response.
forward void Net_MapInfoDownloadFailed(int user_id); // Sent in response to Net_DownloadMapInfo if the download failed.
native int Net_MapInfoGetNumStages(int response_handle); // Get number of stages.
native int Net_MapInfoGetNumBonuses(int response_handle); // Get number of bonuses.

// Download a list of possible replays for a given zone in a leaderboard.
// See NET_ZONE_MAP, Net_GetStageZone and Net_GetBonusZone to get a zone id from a logical stage number.
// See NET_ANGLE_TYPE constants for angled leaderboards.
// The user id is optional, but can be used to identify parallel requests.
native bool Net_DownloadReplayList(int user_id, int zone_id, int angle_type);
forward void Net_ReplayListDownloadReceived(int user_id, int response_handle); // Sent in response to Net_DownloadReplayList to return the response.
forward void Net_ReplayListDownloadFailed(int user_id); // Sent in response to Net_DownloadReplayList if the download failed.
native int Net_ReplayListGetNum(int response_handle); // Get how many replays there are.
native void Net_ReplayListGetPlayerName(int response_handle, int idx, char[] dest, int dest_size); // Get the player name of a replay.
native int Net_ReplayListGetTime(int response_handle, int idx); // Get the finishing time of a replay.

// Download a particular replay for the given zone and leaderboard.
// The index comes from the zone replay list.
// To get the correct values for these parameters, you should use the result from Net_DownloadMapInfo and Net_DownloadReplayList.
// This will send Net_ReplayDownloadReceived or Net_ReplayDownloadFailed some time in the future when the network call is complete.
// The user id is optional, but can be used to identify parallel requests.
native bool Net_DownloadReplay(int user_id, int zone_id, int angle_type, int index);
forward void Net_ReplayDownloadReceived(int user_id, int response_handle); // Sent in response to Net_DownloadReplay to return the response.
forward void Net_ReplayDownloadFailed(int user_id); // Sent in response to Net_DownloadReplay if the download failed.
native bool Net_ReplayDownloadReadData(int response_handle, any[] items, int num_items, int size); // Read from the response file.
native void Net_ReplayDownloadRewind(int response_handle); // Reset the reading position of the response file.
