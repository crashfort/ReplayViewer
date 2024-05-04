#if defined NAV_INCLUDED
#endinput
#endif
#define NAV_INCLUDED

// Navigation mesh functions.

public void RV_StartNav()
{
    char dummy_nav_path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, dummy_nav_path, sizeof(dummy_nav_path), "%s/dummy.nav", RV_ROOT_PATH);

    // Create a dummy navigation file so it doesn't take ages to load new maps.
    Net_CreateDummyNavForMap(dummy_nav_path);
}
