#if defined MAIN_INCLUDED
#endinput
#endif
#define MAIN_INCLUDED

// Events from SourceMod.

public void RV_PluginStart()
{
    RV_RegisterCmds();
}

public void RV_PluginEnd()
{
}

public void RV_MapStart()
{
    RV_StartCmds();
    RV_StartBots();
}

public void RV_MapEnd()
{
}
