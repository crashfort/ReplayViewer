#if defined MENUS_INCLUDED
#endinput
#endif
#define MENUS_INCLUDED

// Menu functions.

// Add a new item to a menu with a given identifier.
// The identifier is later used to refer to this menu item.
public void RV_MenuAddEnumItem(Handle menu, RVMenuId id, char[] title)
{
    char buf[64];
    IntToString(view_as<int>(id), buf, sizeof(buf));
    AddMenuItem(menu, buf, title);
}

// Get back the identifier for a menu item.
public RVMenuId RV_GetMenuItemId(Menu menu, int idx)
{
    char info[8];
    GetMenuItem(menu, idx, info, sizeof(info));

    int id = StringToInt(info);
    return view_as<RVMenuId>(id);
}

public void RV_ExtractMenuItemTokens(Menu menu, int item, char[][] dest, int num_tokens, int token_size)
{
    char info[256];
    GetMenuItem(menu, item, info, sizeof(info));
    ExplodeString(info, ";", dest, num_tokens, token_size);
}

public int RV_HandleAddReplaysMenu(Menu menu, MenuAction action, int param1, int param2)
{
    if (action == MenuAction_Select)
    {
        int client = param1;
        RVMenuId item = RV_GetMenuItemId(menu, param2);

        switch (item)
        {
            case RV_MENU_ADD_ALL_REPLAYS_FOR_MAP:
            {
                RV_AddAllFileReplaysForMap();
                RV_OpenInterfaceMenu(client);
            }
        }
    }

    else if (action == MenuAction_Cancel)
    {
        int client = param1;
        RV_OpenInterfaceMenu(client);
    }

    else if (action == MenuAction_End)
    {
        delete menu;
    }

    return 0;
}

// Options for adding new replays.
public void RV_OpenAddReplaysMenu(int client)
{
    Menu menu = CreateMenu(RV_HandleAddReplaysMenu);
    SetMenuTitle(menu, "Add replays");

    RV_MenuAddEnumItem(menu, RV_MENU_ADD_ALL_REPLAYS_FOR_MAP, "All replays for map");

    DisplayMenu(menu, client, MENU_TIME_FOREVER);
}

public int RV_HandleInterfaceMenu(Menu menu, MenuAction action, int param1, int param2)
{
    if (action == MenuAction_Select)
    {
        int client = param1;
        RVMenuId item = RV_GetMenuItemId(menu, param2);

        switch (item)
        {
            case RV_MENU_ADD_REPLAYS:
            {
                RV_OpenAddReplaysMenu(client);
            }

            case RV_MENU_START_PLAYBACK:
            {
                RV_StartPlayback();
                RV_OpenInterfaceMenu(client);
            }

            case RV_MENU_STOP_PLAYBACK:
            {
                RV_StopPlayback();
                RV_OpenInterfaceMenu(client);
            }

            case RV_MENU_RESET_PLAYBACK:
            {
                RV_ResetPlayback();
                RV_OpenInterfaceMenu(client);
            }

            case RV_MENU_SHOW_REPLAY_TIMES:
            {
                RV_ShowReplayTimes();
                RV_OpenInterfaceMenu(client);
            }
        }
    }

    else if (action == MenuAction_Cancel)
    {
    }

    else if (action == MenuAction_End)
    {
        delete menu;
    }

    return 0;
}

// Main user interaction with the system.
public void RV_OpenInterfaceMenu(int client)
{
    Menu menu = CreateMenu(RV_HandleInterfaceMenu);
    SetMenuTitle(menu, "Replay Viewer");

    // Access state should be synchronized with rv_cmds.inc.

    switch (rv_sys_state)
    {
        case RV_SYS_IDLE:
        {
            RV_MenuAddEnumItem(menu, RV_MENU_ADD_REPLAYS, "Add replays");
        }

        case RV_SYS_REPLAYS_LOADED:
        {
            RV_MenuAddEnumItem(menu, RV_MENU_START_PLAYBACK, "Start playback");
            RV_MenuAddEnumItem(menu, RV_MENU_STOP_PLAYBACK, "Stop playback");
            RV_MenuAddEnumItem(menu, RV_MENU_SHOW_REPLAY_TIMES, "Show times");
        }

        case RV_SYS_PLAYING:
        {
            RV_MenuAddEnumItem(menu, RV_MENU_RESET_PLAYBACK, "Reset playback");
            RV_MenuAddEnumItem(menu, RV_MENU_STOP_PLAYBACK, "Stop playback");
        }

        case RV_SYS_PAUSED:
        {
            RV_MenuAddEnumItem(menu, RV_MENU_START_PLAYBACK, "Start playback");
            RV_MenuAddEnumItem(menu, RV_MENU_STOP_PLAYBACK, "Stop playback");
            RV_MenuAddEnumItem(menu, RV_MENU_SHOW_REPLAY_TIMES, "Show times");
        }
    }

    DisplayMenu(menu, client, MENU_TIME_FOREVER);
}

public int RV_HandleWaitMenu(Menu menu, MenuAction action, int param1, int param2)
{
    return 0;
}

public void RV_CreateWaitMenu()
{
    rv_wait_menu = CreateMenu(RV_HandleWaitMenu);
    SetMenuTitle(rv_wait_menu, "Loading");
}

// Called on map start.
public void RV_StartMenus()
{
    RV_CreateWaitMenu();

    // Download map info immediately.
    // This will call into Net_MapInfoDownloadReceived or Net_MapInfoDownloadFailed in rv_net_callbacks.inc.
    Net_DownloadMapInfo();
}

// Called on map end.
public void RV_StopMenus()
{
    for (int i = 0; i < NET_ZONES_MAX; i++)
    {
        delete rv_zone_menus[i];
    }

    delete rv_zone_list_menu;
    delete rv_wait_menu;
}

public int RV_HandleSelectZoneMenu(Menu menu, MenuAction action, int param1, int param2)
{
    // User has selected a zone to view the replays for, such as stage 6.
    if (action == MenuAction_Select)
    {
        char tokens[2][32];
        RV_ExtractMenuItemTokens(menu, param2, tokens, sizeof(tokens), sizeof(tokens[]));

        int angle_type = StringToInt(tokens[0]);
        int zone_id = StringToInt(tokens[1]);

        // Download the replay list if we have to.
        // This will call into Net_ReplayListDownloadReceived or Net_ReplayListDownloadFailed in rv_net_callbacks.inc.
        if (rv_zone_menus[zone_id] == null)
        {
            DisplayMenu(rv_wait_menu, param1, MENU_TIME_FOREVER);
            Net_DownloadReplayList(param1, zone_id, angle_type);
        }

        else
        {
            DisplayMenu(rv_zone_menus[zone_id], param1, MENU_TIME_FOREVER);
        }
    }

    else if (action == MenuAction_Cancel)
    {
    }

    else if (action == MenuAction_End)
    {
    }

    return 0;
}

// Build the zone list menu now when we have the number of stages and bonuses.
public void RV_CreateZoneListMenu(int response_handle)
{
    int num_stages = Net_MapInfoGetNumStages(response_handle);
    int num_bonuses = Net_MapInfoGetNumBonuses(response_handle);

    rv_zone_list_menu = CreateMenu(RV_HandleSelectZoneMenu);
    SetMenuTitle(rv_zone_list_menu, "Select zone");

    char display[64];
    char info[RV_REPLAY_ID_SIZE];

    // Only forward angle type for now because this is hella messy already.

    Format(info, sizeof(info), "%d;%d", NET_ANGLE_TYPE_FW, NET_ZONE_MAP);
    AddMenuItem(rv_zone_list_menu, info, "Map");

    for (int i = 0; i < num_stages; i++)
    {
        int zone_id = Net_GetStageZone(i);

        Format(display, sizeof(display), "Stage %d", i + 1);
        Format(info, sizeof(info), "%d;%d", NET_ANGLE_TYPE_FW, zone_id);

        AddMenuItem(rv_zone_list_menu, info, display);
    }

    for (int i = 0; i < num_bonuses; i++)
    {
        int zone_id = Net_GetBonusZone(i);

        Format(display, sizeof(display), "Bonus %d", i + 1);
        Format(info, sizeof(info), "%d;%d", NET_ANGLE_TYPE_FW, zone_id);

        AddMenuItem(rv_zone_list_menu, info, display);
    }
}

public int RV_HandleSelectReplayMenu(Menu menu, MenuAction action, int param1, int param2)
{
    // User has selected a replay within a zone.
    if (action == MenuAction_Select)
    {
        char tokens[3][32];
        RV_ExtractMenuItemTokens(menu, param2, tokens, sizeof(tokens), sizeof(tokens[]));

        int angle_type = StringToInt(tokens[0]);
        int zone_id = StringToInt(tokens[1]);
        int replay_rank = StringToInt(tokens[2]);

        char stream_id[RV_REPLAY_ID_SIZE];
        RV_MakeReplayIdentifier(angle_type, zone_id, replay_rank, stream_id, sizeof(stream_id));

        char load_path[PLATFORM_MAX_PATH];
        RV_GetReplayFilePath(stream_id, load_path, sizeof(load_path));

        // Download the replay if we have to.
        // This will call into Net_ReplayDownloadReceived or Net_ReplayDownloadFailed in rv_net_callbacks.inc.
        if (!FileExists(load_path))
        {
            DisplayMenu(rv_wait_menu, param1, MENU_TIME_FOREVER);
            Net_DownloadReplay(param1, zone_id, angle_type, replay_rank);
        }

        else
        {
            RV_LoadStreamSpawnBot(stream_id);
        }
    }

    else if (action == MenuAction_Cancel)
    {
    }

    else if (action == MenuAction_End)
    {
    }

    return 0;
}

// Build the replay selection menu for a given zone.
public void RV_CreateNewReplayListMenu(int response_handle)
{
    int user_id = Net_ReplayListGetUserId(response_handle);
    int angle_type = Net_ReplayListGetAngleType(response_handle);
    int zone_id = Net_ReplayListGetZoneId(response_handle);
    int num_replays = Net_ReplayListGetNum(response_handle);

    Menu new_menu = CreateMenu(RV_HandleSelectReplayMenu);

    char display[256];
    char info[RV_REPLAY_ID_SIZE];
    char time_display[32];
    char name_display[MAX_NAME_LENGTH];

    for (int i = 0; i < num_replays; i++)
    {
        Net_ReplayListGetPlayerName(response_handle, i, name_display, sizeof(name_display));
        int time = Net_ReplayListGetTime(response_handle, i);
        int rank = Net_ReplayListGetRank(response_handle, i); // Rank needed for future download, also nice to display.

        Format(time_display, sizeof(time_display), "%d", time); // TODO Don't know what unit we are getting from the server.
        Format(display, sizeof(display), "%s - %s", time_display, name_display);

        RV_MakeReplayIdentifier(angle_type, zone_id, rank, info, sizeof(info));

        AddMenuItem(new_menu, info, display);
    }

    rv_zone_menus[zone_id] = new_menu;

    // Stop the waiting menu and open the expected menu.
    DisplayMenu(new_menu, user_id, MENU_TIME_FOREVER);
}
