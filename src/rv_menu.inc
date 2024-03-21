#if defined MENUS_INCLUDED
#endinput
#endif
#define MENUS_INCLUDED

// Menu functions.

// Add a new item to a menu with a given identifier.
// The identifier is later used to refer to this menu item.
public void RV_MenuAddEnumItem(Handle menu, RvMenuId id, char[] title)
{
    char buf[64];
    IntToString(view_as<int>(id), buf, sizeof(buf));
    AddMenuItem(menu, buf, title);
}

// Get back the identifier for a menu item.
public RvMenuId RV_GetMenuItemId(Menu menu, int idx)
{
    char info[8];
    GetMenuItem(menu, idx, info, sizeof(info));

    int id = StringToInt(info);
    return view_as<RvMenuId>(id);
}

// Instead of managing a huge list with many pages of hard to read names,
// let the file management be done in the file explorer and just add everything in there.
public void RV_AddAllReplaysForMap(int client)
{
    char dir_path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, dir_path, sizeof(dir_path), "%s/replays/%s", RV_ROOT_PATH, RV_CurrentMap);

    if (!DirExists(dir_path))
    {
        PrintToChat(client, "No replays exist for map %s", RV_CurrentMap);
        return;
    }

    DirectoryListing dir_lister = OpenDirectory(dir_path);

    if (dir_lister == null)
    {
        PrintToChat(client, "Could not open directory for map %s", RV_CurrentMap);
        return;
    }

    FileType file_type;
    char file_name[PLATFORM_MAX_PATH];
    int index = 0;

    char full_path[PLATFORM_MAX_PATH];

    while (ReadDirEntry(dir_lister, file_name, sizeof(file_name), file_type))
    {
        if (file_type != FileType_File)
        {
            continue;
        }

        FormatEx(full_path, sizeof(full_path), "%s/%s", dir_path, file_name);

        RV_SpawnBotWithStream(full_path);

        index++;
    }

    delete dir_lister;

    if (index == 0)
    {
        PrintToChat(client, "No replays available for %s", RV_CurrentMap);
        return;
    }

    RV_PlaybackActive = true;
}

public int RV_HandleAddReplaysMenu(Menu menu, MenuAction action, int param1, int param2)
{
    if (action == MenuAction_Select)
    {
        int client = param1;
        RvMenuId item = RV_GetMenuItemId(menu, param2);

        switch (item)
        {
            case RV_MENU_ADD_ALL_REPLAYS_FOR_MAP:
            {
                RV_AddAllReplaysForMap(client);
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
        RvMenuId item = RV_GetMenuItemId(menu, param2);

        switch (item)
        {
            case RV_MENU_ADD_REPLAYS:
            {
                RV_OpenAddReplaysMenu(client);
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

    RV_MenuAddEnumItem(menu, RV_MENU_ADD_REPLAYS, "Add replays");

    DisplayMenu(menu, client, MENU_TIME_FOREVER);
}
