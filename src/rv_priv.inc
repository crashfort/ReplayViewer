#if defined PRIV_INCLUDED
#endinput
#endif
#define PRIV_INCLUDED

// Structures and constants and variables.

#define RV_ROOT_PATH "data/replay_viewer"

// Size of the key used to identify a replay, in chars.
#define RV_REPLAY_ID_SIZE 64

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
    float angles[3];
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
    RV_BOT_FINISHED,
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
    RV_MENU_START_PLAYBACK,
    RV_MENU_STOP_PLAYBACK,
    RV_MENU_RESET_PLAYBACK,
    RV_MENU_LIST_ZONES,
}

// Information about a replay.
enum struct RVReplayListing
{
    char id[RV_REPLAY_ID_SIZE]; // Unique replay identifier.
    char name[MAX_NAME_LENGTH]; // Player name.
    char time[64]; // Finishing time. Already formatted.
    char date[64]; // Record date. Already formatted.
    int angle_type; // Forwards or sideways etc, see NET_ANGLE_TYPE constants.
    int zone_id; // Zone identifier, see NET_ZONE constants.
}

// -----------------------------------
// System state:

// References to every bot.
RVBot rv_bots[MAXPLAYERS + 1]; // Using indexes 0 and up.
int rv_num_bots; // How many bots are actually placed in rv_bots.
int rv_bot_maps[MAXPLAYERS + 1]; // Remaps from client ids to bot indexes.

char rv_cur_map[128];

// Downloaded replay listings for a zone.
// These are created on demand and reused, meaning that new requests are not made for existing ones.
StringMap rv_replay_listings; // Value is RVReplayListing.

// Main menu.
// This is created once after map start.
Menu rv_main_menu;

// Menu for showing what zones exist in the map.
// This is created once after map start, when the map info is received.
Menu rv_zone_list_menu;

// Menus for replay selection in a zone.
// These are created on demand and reused, meaning that new requests are not made for existing ones.
Menu rv_zone_menus[NET_ZONES_MAX];

// Currently playing streams.
// Used to guard against duplication during playback in the replay selection menu.
StringMap rv_loaded_streams; // Value is int.
