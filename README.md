# ReplayViewer

This allows you to view downloaded KSF replays played on Counter-Strike: Source in a local server. Many replays can be played at once and remain synchronized with each other, allowing you to spectate individually.

Compatible with [SVR](https://github.com/crashfort/SourceDemoRender/), which can be used to create historical record videos or record comparison videos where you need to have more simultaneous players. The replay viewer will also produce smoother and more accurate playback without teleport lag, compared to traditional interpolated server side demo playback.

## User instructions

Steps 1 to 3 only have to be done once, or when respective programs have to update.

1. Download [SourceMod](https://www.sourcemod.net/downloads.php) and [Metamod](https://www.metamodsource.net/).
2. Extract **SourceMod** and **Metamod** archives into `cstrike/`.
3. Extract **ReplayViewer** archive from the [releases page](https://github.com/crashfort/ReplayViewer/releases) into `cstrike/`.
4. Start the game with `-insecure` or start with `svr_launcher.exe` from [SVR](https://github.com/crashfort/SourceDemoRender/).
5. Start the map you have replays for using the console command `map <name>`.
6. Place new replays into `cstrike/addons/sourcemod/data/replay_viewer/replays/<map name>/`.
7. Use the chat command `/replay_viewer` to open the interface. Alternatively use the console commands below.
8. Load all the replays you want to play using the interface
9. Control playback using the interface.
10. In spectator mode, you can now use `startmovie` from [SVR](https://github.com/crashfort/SourceDemoRender/).

## Chat commands

You can use the menu interface, or the commands below to control the replay viewer. **You must type these in the chat and not the console**, like this:
```
/replay_viewer
```

| Command | Description
| ------------- | -----------------------
| **replay_viewer** | Open the user interface.
| **load_all_replays** | Load all replays for the current map. All replays located in `cstrike/addons/sourcemod/data/replay_viewer/replays/<map name>/` will be loaded. No replays must be loaded before this can be used.
| **start_playback** | Start the playback of all loaded replays. Replays must have been loaded before this can be used.
| **stop_playback** | Stop and remove all replays. Replays must have been loaded before this can be used.
| **reset_playback** | Stop playback and return all replays to the start. Replays must have been loaded before this can be used.

## Building the SourcePawn code

1. Compile `replay_viewer.sp` with `cstrike\addons\sourcemod\scripting\spcomp.exe`.

## Building the C++ extension code

1. Following [AlliedModders recommendation](https://wiki.alliedmods.net/Writing_Extensions#Environment_Variables):
2. Clone [SourceMod](https://github.com/alliedmodders/sourcemod/), [Metamod](https://github.com/alliedmodders/metamod-source/) and [HL2 SDK](https://github.com/alliedmodders/hl2sdk).
3. Set `HL2SDKCSS` to the cloned path of `HL2 SDK`.
4. Set `MMSOURCE19` to the cloned path of `Metamod`.
5. Set `SMSOURCE` to the cloned path of `SourceMod`.
6. Open `extensions.sln`.
