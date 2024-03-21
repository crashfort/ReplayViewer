#if defined PRIV_INCLUDED
#endinput
#endif
#define PRIV_INCLUDED

// Structures and constants and variables.

#define RV_ROOT_PATH "data/replay_viewer"

// One frame of the replay stream.
enum struct RVFrame
{
    int buttons;
    float position[3];
    float angles[2];
    float velocity[3];
}

enum struct RVStream
{
    int start_offset;
    int num_frames;
    ArrayList frames; // Array of RVFrame.
}

// One fake player that will be controlled by the replay stream.
enum struct RVBot
{
    int id; // Identifier used for client identification.
    int playback_frame; // What index in stream.frames.
    RVStream stream; // Stream we are reading from.
}

// Constants used to refer to menu items.
enum RvMenuId
{
    RV_MENU_TOGGLE_PLAY,
    RV_MENU_ADD_REPLAYS,
    RV_MENU_ADD_ALL_REPLAYS_FOR_MAP,
    RV_MENU_START_PLAYBACK,
    RV_MENU_STOP_PLAYBACK,
    RV_MENU_RESET_PLAYBACK,
};

// References to every bot.
RVBot RV_Bots[MAXPLAYERS + 1];
int RV_NumBots;
int RV_BotMaps[MAXPLAYERS + 1];

char RV_CurrentMap[128];

bool RV_PlaybackActive; // True if playback is currently happening.
bool RV_HaveBotsPlaying; // True if bots are spawned and in a team. Though they may not be playing back.
