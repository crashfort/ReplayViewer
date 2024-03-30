#if defined NET_INCLUDED
#endinput
#endif
#define NET_INCLUDED

// Internet functions.

// Returns true if a connection to the Internet is available right now.
// Calls to other net functions should not be done if this returns false.
native bool Net_ConnectedToInet();

// Get the real name of a player from an internal player id.
// This will send Net_PlayerNameReceived or Net_PlayerNameFailed some time in the future when the network call is complete.
// Returns false if no connection is available.
native bool Net_RequestPlayerName(int player_id);

// Sent in response to Net_RequestPlayerName to return the name of a player.
forward void Net_PlayerNameReceived(int player_id, char[] name);

// Sent in response to Net_RequestPlayerName if the download request failed.
forward void Net_PlayerNameFailed(int player_id);

// Download a ranked replay for the current map.
// This will send Net_ReplayDownloadReceived or Net_ReplayDownloadFailed some time in the future when the network call is complete.
// Returns false if no connection is available.
native bool Net_RequestReplay(int rank);

// Sent in response to Net_RequestReplay to return the downloaded replay.
forward void Net_ReplayDownloadReceived(int rank, char[] file_path);

// Sent in response to Net_RequestReplay if the download request failed.
forward void Net_ReplayDownloadFailed(int rank);
