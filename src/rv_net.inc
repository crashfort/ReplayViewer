#if defined NET_INCLUDED
#endinput
#endif
#define NET_INCLUDED

// Internet functions.

// Zone 0 is the main map, 1 - 30 are stages and 31 - 40 are bonuses.
#define NET_ZONE_MAP 0
#define NET_ZONE_FIRST_STAGE 1
#define NET_ZONE_FIRST_BONUS 31
#define NET_ZONES_MAX 40

#define NET_ANGLE_TYPE_FW 0
#define NET_ANGLE_TYPE_SW 1
#define NET_ANGLE_TYPE_HSW 2
#define NET_ANGLE_TYPE_BW 3
#define NET_ANGLE_TYPES_MAX 4

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

stock void Net_CategorizeZoneId(int zone_id, char[] dest, int dest_size)
{
    if (zone_id == NET_ZONE_MAP)
    {
        Format(dest, dest_size, "Map");
        return;
    }

    if (zone_id >= NET_ZONE_FIRST_BONUS)
    {
        Format(dest, dest_size, "Bonus %d", (zone_id - NET_ZONE_FIRST_BONUS) + 1);
        return;
    }

    if (zone_id >= NET_ZONE_FIRST_STAGE)
    {
        Format(dest, dest_size, "Stage %d", (zone_id - NET_ZONE_FIRST_STAGE) + 1);
        return;
    }
}

// The response handles are only valid to be used in the functions that pass them.
// Some functions allow to pass a user id. This is very useful if you are working with menus to refer
// to the client, when you need to open a menu again after the network download.

// Returns true if a connection to the Internet is available right now.
// Calls to other net functions can not be done if this returns false.
native bool Net_ConnectedToInet();

// Get information about the current map, such as the zones.
native bool Net_DownloadMapInfo();

forward void Net_MapInfoDownloadReceived(int response_handle); // Sent in response to Net_DownloadMapInfo to return the response.
forward void Net_MapInfoDownloadFailed(); // Sent in response to Net_DownloadMapInfo if the download failed.

native int Net_MapInfoGetNumStages(int response_handle); // Get number of stages.
native int Net_MapInfoGetNumBonuses(int response_handle); // Get number of bonuses.
native bool Net_MapInfoGetLinear(int response_handle); // Get staged or linear map.

// Download a list of possible replays for a given zone in a leaderboard.
// See NET_ZONE_MAP, Net_GetStageZone and Net_GetBonusZone to get a zone id from a logical stage number.
// See NET_ANGLE_TYPE constants for angled leaderboards.
// The user id is optional, but can be used to identify requests in the response handler.
native bool Net_DownloadReplayList(int user_id, int zone_id, int angle_type);

forward void Net_ReplayListDownloadReceived(int response_handle); // Sent in response to Net_DownloadReplayList to return the response.
forward void Net_ReplayListDownloadFailed(); // Sent in response to Net_DownloadReplayList if the download failed.

native int Net_ReplayListGetUserId(int response_handle); // Get the user id passed in the request.
native int Net_ReplayListGetNum(int response_handle); // Get how many replays there are.
native void Net_ReplayListGetId(int response_handle, int idx, char[] dest, int dest_size); // Get the unique identifier of a replay.
native void Net_ReplayListGetName(int response_handle, int idx, char[] dest, int dest_size); // Get the player name of a replay.
native void Net_ReplayListGetTime(int response_handle, int idx, char[] dest, int dest_size); // Get the finishing time of a replay.
native void Net_ReplayListGetDate(int response_handle, int idx, char[] dest, int dest_size); // Get the record date of a replay.
native int Net_ReplayListGetZoneId(int response_handle); // Get the zone id of a replay.
native int Net_ReplayListGetAngleType(int response_handle); // Get the angle type of a replay.

// Download a particular replay for the given zone and leaderboard.
// The replay index comes from the zone replay list.
// To get the correct values for these parameters, you should use the result from Net_DownloadMapInfo and Net_DownloadReplayList.
// This will send Net_ReplayDownloadReceived or Net_ReplayDownloadFailed some time in the future when the network call is complete.
// The user id is optional, but can be used to identify requests in the response handler.
native bool Net_DownloadReplay(int user_id, char[] name);

forward void Net_ReplayDownloadReceived(int response_handle); // Sent in response to Net_DownloadReplay to return the response.
forward void Net_ReplayDownloadFailed(); // Sent in response to Net_DownloadReplay if the download failed.

native int Net_ReplayDownloadGetUserId(int response_handle); // Get the user id passed in the request.
native bool Net_ReplayDownloadWriteToFile(int response_handle, char[] dest); // Write the replay file to file.
native int Net_ReplayDownloadGetId(int response_handle, char[] dest, int dest_size); // Get the unique identifier of a replay.
