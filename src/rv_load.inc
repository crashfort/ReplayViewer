#if defined LOAD_INCLUDED
#endinput
#endif
#define LOAD_INCLUDED

#define RV_MAGIC 0x02 // Identifier or version.

bool RV_LoadFileStream(char[] path, RVStream stream)
{
    File file = OpenFile(path, "rb");

    if (file == null)
    {
        PrintToChatAll("Could not load stream \"%s\"", path);
        return false;
    }

    // PrintToChatAll("Loaded stream \"%s\"", path);

    RVHeader header;
    ReadFile(file, header, sizeof(header), 4);

    if (header.magic != RV_MAGIC)
    {
        PrintToChatAll("Could not load stream \"%s\", wrong version or wrong file type (%d), expected %d", path, header.magic, RV_MAGIC);
        delete file;
        return false;
    }

    stream.bookmarks = CreateArray(sizeof(RVBookmark));
    ResizeArray(stream.bookmarks, header.num_bookmarks);

    for (int i = 0; i < header.num_bookmarks; i++)
    {
        RVBookmark bk;
        ReadFile(file, bk, sizeof(bk), 4);

        SetArrayArray(stream.bookmarks, i, bk);
    }

    stream.frames = CreateArray(sizeof(RVFrame));
    ResizeArray(stream.frames, header.num_frames);

    for (int i = 0; i < header.num_frames; i++)
    {
        RVFrame frame;
        ReadFile(file, frame, sizeof(frame), 4);
        SetArrayArray(stream.frames, i, frame);
    }

    stream.num_frames = header.num_frames;
    stream.num_bookmarks = header.num_bookmarks;
    stream.start_offset = 0;
    stream.team = header.team;

    // Find the start offset so we can synchronize the start of all replays.

    for (int i = 0; i < stream.num_bookmarks; i++)
    {
        RVBookmark bk;
        GetArrayArray(stream.bookmarks, i, bk);

        // First event of this type will be the true start.
        if (bk.type == RV_BK_START)
        {
            stream.start_offset = bk.frame;
            break;
        }
    }

    // Find the end offset so we can set the real duration.

    for (int i = stream.num_bookmarks - 1; i >= 0; i--)
    {
        RVBookmark bk;
        GetArrayArray(stream.bookmarks, i, bk);

        // Last event of this type will be the true end.
        if (bk.type == RV_BK_STOP)
        {
            stream.end_offset = stream.num_frames - bk.frame;
            break;
        }
    }

    stream.duration = stream.num_frames - stream.start_offset - stream.end_offset;

    delete file;

    return true;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!
// Code unused for now, offline playback would need more menu work to work good.
// !!!!!!!!!!!!!!!!!!!!!!!!!

// Instead of managing a huge list with many pages of hard to read names,
// let the file management be done in the file explorer and just add everything in there.
public void RV_AddAllFileReplaysForMap()
{
    char dir_path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, dir_path, sizeof(dir_path), "%s/replays/%s", RV_ROOT_PATH, rv_cur_map);

    if (!DirExists(dir_path))
    {
        PrintToChatAll("No replays exist for map %s", rv_cur_map);
        return;
    }

    DirectoryListing dir_lister = OpenDirectory(dir_path);

    if (dir_lister == null)
    {
        PrintToChatAll("Could not open directory for map %s", rv_cur_map);
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

        if (!RV_CanSpawnMoreBots())
        {
            break; // We are full.
        }

        FormatEx(full_path, sizeof(full_path), "%s/%s", dir_path, file_name);

        RVStream stream;

        if (!RV_LoadFileStream(full_path, stream))
        {
            continue;
        }

        // For files, we do not have any way to get the player name.
        // Use the file name and let the user make good names.
        RV_SpawnBotWithStream(stream, file_name);

        index++;
    }

    delete dir_lister;

    if (index == 0)
    {
        PrintToChatAll("No replays available for %s", rv_cur_map);
        return;
    }
}

public void RV_FreeStream(RVStream stream)
{
    if (stream.frames != null)
    {
        delete stream.frames;
        stream.frames = null;
    }

    if (stream.bookmarks != null)
    {
        delete stream.bookmarks;
        stream.bookmarks = null;
    }
}

public void RV_DupeStream(RVStream source, RVStream dest)
{
    dest = source;
    dest.bookmarks = RV_CloneArrayHandle(source.bookmarks);
    dest.frames = RV_CloneArrayHandle(source.frames);
}

// Called on map start.
public void RV_StartLoad()
{
    rv_replay_listings = CreateTrie();
}

// Called on map end.
public void RV_StopLoad()
{
    delete rv_replay_listings;
}

// Store data from the replay listing.
public void RV_ProcessDownloadedListing(int response_handle)
{
    int angle_type = Net_ReplayListGetAngleType(response_handle);
    int zone_id = Net_ReplayListGetZoneId(response_handle);
    int num_replays = Net_ReplayListGetNum(response_handle);

    char listing_id[RV_REPLAY_ID_SIZE];

    for (int i = 0; i < num_replays; i++)
    {
        Net_ReplayListGetId(response_handle, i, listing_id, sizeof(listing_id));

        RVReplayListing listing;
        strcopy(listing.id, sizeof(listing.id), listing_id);
        Net_ReplayListGetName(response_handle, i, listing.name, sizeof(listing.name));
        Net_ReplayListGetTime(response_handle, i, listing.time, sizeof(listing.time));
        Net_ReplayListGetDate(response_handle, i, listing.date, sizeof(listing.date));
        listing.angle_type = angle_type;
        listing.zone_id = zone_id;

        SetTrieArray(rv_replay_listings, listing_id, listing, sizeof(listing));
    }
}

// Get full path to a replay file in the current map.
public void RV_GetReplayFilePath(char[] stream_id, char[] dest, int dest_size)
{
    BuildPath(Path_SM, dest, dest_size, "%s/replays/%s/%s", RV_ROOT_PATH, rv_cur_map, stream_id);
}

// Load a downloaded replay and spawn a bot with that stream.
public bool RV_LoadStreamSpawnBot(char[] stream_id)
{
    char load_path[PLATFORM_MAX_PATH];
    RV_GetReplayFilePath(stream_id, load_path, sizeof(load_path));

    if (!FileExists(load_path))
    {
        return false;
    }

    RVStream stream;

    if (!RV_LoadFileStream(load_path, stream))
    {
        return false;
    }

    // This function is only to be used when downloading replays, in which case the listings must exist.
    // The offline file loadings above do not care about the listings.

    RVReplayListing listing;

    if (!GetTrieArray(rv_replay_listings, stream_id, listing, sizeof(listing)))
    {
        return false;
    }

    char name[256];
    Format(name, sizeof(name), "[%s] %s (%s)", listing.time, listing.name, listing.date);

    bool res = RV_SpawnBotWithStream(stream, name);
    return res;
}

// A replay selected in the zone menu has been downloaded.
// This saves the replay to file for future cache, and then loads that file and spawns a bot with that stream.
public void RV_ProcessDownloadedReplay(int response_handle)
{
    char stream_id[RV_REPLAY_ID_SIZE];
    Net_ReplayDownloadGetId(response_handle, stream_id, sizeof(stream_id));

    PrintToServer("Downloaded %s", stream_id);

    // Save the stream to file and then load that.
    // This makes it a lot easier because the code for loading a file already exists.
    // Reading from memory is just super annoying to do in SourcePawn, and it would be stupid
    // to have duplicate code for reading the same format from different mediums.

    // Maybe create new directory for this map.

    char dir_path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, dir_path, sizeof(dir_path), "%s/replays/%s", RV_ROOT_PATH, rv_cur_map);
    RV_MaybeCreateDir(dir_path);

    char save_path[PLATFORM_MAX_PATH];
    RV_GetReplayFilePath(stream_id, save_path, sizeof(save_path));

    Net_ReplayDownloadWriteToFile(response_handle, save_path);

    RV_LoadStreamSpawnBot(stream_id);
}
