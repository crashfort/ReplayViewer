# ReplayViewer

This allows you to render KSF replays played on Counter-Strike: Source in a local server compatible with [SVR](https://github.com/crashfort/SourceDemoRender/). The replay viewer will produce smoother and more accurate playback without teleport lag, compared to traditional interpolated server side demo playback. Sequences of replays can be automatically rendered using [SVR Studio]((https://github.com/crashfort/SourceDemoRender/)).

## User instructions

Steps 1 to 3 only have to be done once, or when respective programs have to update.

1. Download [SourceMod](https://www.sourcemod.net/downloads.php) and [Metamod](https://www.metamodsource.net/).
2. Extract **SourceMod** and **Metamod** archives into `cstrike/`.
3. Extract **ReplayViewer** archive from the [releases page](https://github.com/crashfort/ReplayViewer/releases) into `cstrike/`.
4. Start the game with [SVR Studio](https://github.com/crashfort/SourceDemoRender/).

## Building the SourcePawn code

1. Compile `replay_viewer.sp` with `cstrike\addons\sourcemod\scripting\spcomp.exe`.

## Building the C++ extension code

Only 64 bit is supported.

1. Following [AlliedModders environment variables recommendation](https://wiki.alliedmods.net/Writing_Extensions#Environment_Variables):
2. Clone [SourceMod](https://github.com/alliedmodders/sourcemod/), [Metamod](https://github.com/alliedmodders/metamod-source/) and [HL2 SDK](https://github.com/alliedmodders/hl2sdk).
3. Set `HL2SDKCSS` to the cloned path of `HL2 SDK`.
4. Set `MMSOURCE19` to the cloned path of `Metamod`.
5. Set `SMSOURCE` to the cloned path of `SourceMod`.
6. Build `deps\bzip2\bzip2.sln` in **Release x64**.
7. Build `extensions.sln`.
