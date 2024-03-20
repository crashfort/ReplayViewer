#if defined MENUS_INCLUDED
#endinput
#endif
#define MENUS_INCLUDED

// Menu functions.

// Add a new item to a menu with a given identifier.
// Converts the enum value "data" into a string and sets it as the info value to a menu item.
public void RV_MenuAddEnumEntry(Handle menu, RvMenuId id, char[] title)
{
    char buf[64];
    IntToString(view_as<int>(id), buf, sizeof(buf));
    AddMenuItem(menu, buf, title);
}

public int RV_HandleInterfaceMenu(Menu menu, MenuAction action, int param1, int param2)
{
    if (action == MenuAction_Select)
    {
        int client = param1;
        int item = param2;
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

    // Need to build the menu.

    DisplayMenu(menu, client, MENU_TIME_FOREVER);
}
