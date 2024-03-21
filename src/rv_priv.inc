#if defined PRIV_INCLUDED
#endinput
#endif
#define PRIV_INCLUDED

// Structures and constants and variables.

#define RV_ROOT_PATH "data/replay_viewer"

// Static information about the replay stream.
enum struct RVHeader
{
    int version; // Format version.
    int length; // How many frames there are.
    char name[MAX_NAME_LENGTH]; // Name of player.
    int team; // What team the bot should start in.
}

// One frame of the replay stream.
enum struct RVFrame
{
    int buttons;
    float position[3];
    float angles[2];
    float velocity[3];
}

// One fake player that will be controlled by the replay stream.
enum struct RVBot
{
    int id; // Identifier used for various functions.
    int playback_tick; // What index in frames.
    int playback_length; // How many frames there are.
    int start_offset; // Adjustments for sync.
    ArrayList frames; // Array of RVFrame.
}

// Constants used to refer to menu items.
enum RvMenuId
{
    RV_MENU_TOGGLE_PLAY,
    RV_MENU_ADD_REPLAYS,
    RV_MENU_ADD_ALL_REPLAYS_FOR_MAP,
    RV_MENU_CLEAR,
    RV_MENU_OFFSET_REPLAY,
};

// References to every bot.
RVBot RV_Bots[MAXPLAYERS + 1];

char RV_CurrentMap[128];

bool RV_PlaybackActive;
