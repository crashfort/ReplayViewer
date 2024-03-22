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
                RV_AddAllReplaysForMap(client);
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

    switch (RV_SystemState)
    {
        case RV_SYS_IDLE:
        {
            RV_MenuAddEnumItem(menu, RV_MENU_ADD_REPLAYS, "Add replays");
        }

        case RV_SYS_REPLAYS_LOADED:
        {
            RV_MenuAddEnumItem(menu, RV_MENU_START_PLAYBACK, "Start playback");
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
        }
    }

    DisplayMenu(menu, client, MENU_TIME_FOREVER);
}
