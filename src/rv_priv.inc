#if defined PRIV_INCLUDED
#endinput
#endif
#define PRIV_INCLUDED

// Structures and constants and variables.

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
    float angles[3]; // Roll not used.
    float velocity[3];
}

// One fake player that will be controlled by the replay stream.
enum struct RVBot
{
    int id; // Identifier used for various functions.
    int playback_tick; // What index in frames.
    int start_offset; // Adjustments for sync.
    ArrayList frames; // Array of RVFrame.
}

// Constants used to refer to menu items.
enum RvMenuId
{
    MENU_Play, // Start playing now.
    MENU_AddBot, // Add a new bot.
};

// References to every bot.
RVBot RV_Bots[MAXPLAYERS];
