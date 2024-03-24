#if defined PRIV_INCLUDED
#endinput
#endif
#define PRIV_INCLUDED

// Structures and constants and variables.

#define RV_ROOT_PATH "data/replay_viewer"

// Prefix of files we want to load.
// This is not a restriction, but means the name has additional information we can load.
#define RV_FILE_PREFIX "replay_css_"

enum RVSystemState
{
    RV_SYS_IDLE,
    RV_SYS_REPLAYS_LOADED,
    RV_SYS_PLAYING,
    RV_SYS_PAUSED,
}

enum struct RVFileMeta
{
    int map_id; // Internal map identifier.
    int mode; // Internal finish mode (forwards, sideways etc).
    int player_id; // Internal player identifier.
    int timestamp[2]; // Unix timestamp low and high bits.
}

// Data that we read from the file.
enum struct RVHeader
{
    int magic; // Identifier or version.
    int team; // Team the player was in.
    int num_frames; // How many frames there are.
    int num_bookmarks; // How many events there are.
}

// One frame of the replay stream.
// Data that we read from the file.
enum struct RVFrame
{
    int buttons; // See IN_ constants.
    float position[3];
    float angles[2];
    int reserved;
    float velocity[3];
}

enum RVBookmarkType
{
    RV_BK_NONE,
    RV_BK_CONTINUE,
    RV_BK_STOP,
    RV_BK_START,
}

// Data that we read from the file.
enum struct RVBookmark
{
    int frame; // Frame onwards where this event is active.
    RVBookmarkType type; // Event type.
    int stage; // Stage or checkpoint.
    char target_name[256]; // Used by maps for trigger filter logic.
    char class_name[256]; // Used by maps for trigger filter logic.
}

enum struct RVStream
{
    int start_offset; // Offset until timing starts, in frames.
    int end_offset; // Offset from the end where timing ends, in frames.
    int duration; // Real timed length, minus start and end offset, in frames. Use RV_FramesToTime to get milliseconds.
    int num_frames; // How many frames there are in total.
    int num_bookmarks; // How many bookmarks there are in total.
    int team; // Team the player was in.
    ArrayList bookmarks; // Event information about stages etc.
    ArrayList frames; // Array of RVFrame.
}

// Bot update state.
enum RVBotState
{
    RV_BOT_IDLE,
    RV_BOT_WAITING,
    RV_BOT_PLAYING,
}

// One fake player that will be controlled by the replay stream.
enum struct RVBot
{
    int id; // Identifier used for client identification.
    int playback_frame; // What index in stream.frames.
    RVBotState state; // Bot update state.
    RVStream stream; // Stream we are reading from.
}

// Constants used to refer to menu items.
enum RVMenuId
{
    RV_MENU_TOGGLE_PLAY,
    RV_MENU_ADD_REPLAYS,
    RV_MENU_ADD_ALL_REPLAYS_FOR_MAP,
    RV_MENU_START_PLAYBACK,
    RV_MENU_STOP_PLAYBACK,
    RV_MENU_RESET_PLAYBACK,
};

// -----------------------------------
// System state:

// References to every bot.
RVBot rv_bots[MAXPLAYERS + 1]; // Using indexes 0 and up.
int rv_num_bots; // How many bots are actually placed in rv_bots.
int rv_bot_maps[MAXPLAYERS + 1]; // Remaps from client ids to bot indexes.

char rv_cur_map[128];

RVSystemState rv_sys_state;
