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

public int RV_HandlePlaybackMenu(Menu menu, MenuAction action, int param1, int param2)
{
    if (action == MenuAction_Select)
    {
        RVMenuId item = RV_GetMenuItemId(menu, param2);

        switch (item)
        {
            case RV_MENU_START_PLAYBACK:
            {
                RV_StartPlayback();
                DisplayMenu(rv_main_menu, param1, MENU_TIME_FOREVER);
            }

            case RV_MENU_STOP_PLAYBACK:
            {
                RV_StopPlayback();
                DisplayMenu(rv_main_menu, param1, MENU_TIME_FOREVER);
            }

            case RV_MENU_RESET_PLAYBACK:
            {
                RV_ResetPlayback();
                DisplayMenu(rv_main_menu, param1, MENU_TIME_FOREVER);
            }

            case RV_MENU_LIST_ZONES:
            {
                DisplayMenu(rv_zone_list_menu, param1, MENU_TIME_FOREVER);
            }
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

public void RV_CreateMainMenu()
{
    rv_main_menu = CreateMenu(RV_HandlePlaybackMenu);
    SetMenuTitle(rv_main_menu, "Replay Viewer");

    RV_MenuAddEnumItem(rv_main_menu, RV_MENU_START_PLAYBACK, "Start playback");
    RV_MenuAddEnumItem(rv_main_menu, RV_MENU_STOP_PLAYBACK, "Stop playback");
    RV_MenuAddEnumItem(rv_main_menu, RV_MENU_RESET_PLAYBACK, "Reset playback");
    RV_MenuAddEnumItem(rv_main_menu, RV_MENU_LIST_ZONES, "List replays");
}

// Called on map start.
public void RV_StartMenus()
{
    RV_CreateMainMenu();

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
    delete rv_main_menu;
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
            Net_DownloadReplayList(param1, zone_id, angle_type);
        }

        else
        {
            DisplayMenu(rv_zone_menus[zone_id], param1, MENU_TIME_FOREVER);
        }
    }

    else if (action == MenuAction_Cancel)
    {
        if (param2 == MenuCancel_ExitBack)
        {
            DisplayMenu(rv_main_menu, param1, MENU_TIME_FOREVER); // Go back to the main menu.
        }
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
    bool linear = Net_MapInfoGetLinear(response_handle);

    rv_zone_list_menu = CreateMenu(RV_HandleSelectZoneMenu);
    SetMenuTitle(rv_zone_list_menu, "Select zone");
    SetMenuExitBackButton(rv_zone_list_menu, true);

    char display[64];
    char info[32];

    // Only forward angle type for now because this is hella messy already.

    Format(info, sizeof(info), "%d;%d", NET_ANGLE_TYPE_FW, NET_ZONE_MAP);
    AddMenuItem(rv_zone_list_menu, info, "Map");

    if (!linear)
    {
        for (int i = 0; i < num_stages; i++)
        {
            int zone_id = Net_GetStageZone(i);

            Format(display, sizeof(display), "Stage %d", i + 1);
            Format(info, sizeof(info), "%d;%d", NET_ANGLE_TYPE_FW, zone_id);

            AddMenuItem(rv_zone_list_menu, info, display);
        }
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
        char replay_id[RV_REPLAY_ID_SIZE];
        GetMenuItem(menu, param2, replay_id, sizeof(replay_id));

        char load_path[PLATFORM_MAX_PATH];
        RV_GetReplayFilePath(replay_id, load_path, sizeof(load_path));

        // Download the replay if we have to.
        // This will call into Net_ReplayDownloadReceived or Net_ReplayDownloadFailed in rv_net_callbacks.inc.
        if (!FileExists(load_path))
        {
            Net_DownloadReplay(param1, replay_id);
        }

        else
        {
            RV_LoadStreamSpawnBot(replay_id);
        }

        DisplayMenu(menu, param1, MENU_TIME_FOREVER);
    }

    else if (action == MenuAction_Cancel)
    {
        if (param2 == MenuCancel_ExitBack)
        {
            DisplayMenu(rv_zone_list_menu, param1, MENU_TIME_FOREVER); // Go back to zone selection.
        }
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
    // int angle_type = Net_ReplayListGetAngleType(response_handle);
    int zone_id = Net_ReplayListGetZoneId(response_handle);
    int num_replays = Net_ReplayListGetNum(response_handle);

    Menu menu = CreateMenu(RV_HandleSelectReplayMenu);
    SetMenuExitBackButton(menu, true);

    char full_row_display[256];
    char time_display[64];
    char date_display[64];
    char name_display[MAX_NAME_LENGTH];
    char replay_id[RV_REPLAY_ID_SIZE];

    for (int i = 0; i < num_replays; i++)
    {
        Net_ReplayListGetName(response_handle, i, name_display, sizeof(name_display));
        Net_ReplayListGetTime(response_handle, i, time_display, sizeof(time_display));
        Net_ReplayListGetDate(response_handle, i, date_display, sizeof(date_display));
        Net_ReplayListGetId(response_handle, i, replay_id, sizeof(replay_id));

        Format(full_row_display, sizeof(full_row_display), "[%s] %s (%s)", time_display, name_display, date_display);

        AddMenuItem(menu, replay_id, full_row_display);
    }

    rv_zone_menus[zone_id] = menu;

    DisplayMenu(menu, user_id, MENU_TIME_FOREVER);
}
