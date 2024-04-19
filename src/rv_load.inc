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

    rv_sys_state = RV_SYS_REPLAYS_LOADED;
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
    char stupid[RV_REPLAY_ID_SIZE];

    rv_replay_listing_ids = CreateArray(sizeof(stupid));
    rv_replay_listings = CreateArray(sizeof(RVReplayListing));
    rv_zone_stream_ids = CreateArray(sizeof(stupid));
    rv_zone_streams = CreateArray(sizeof(RVStream));
}

// Called on map end.
public void RV_StopLoad()
{
    delete rv_replay_listing_ids;
    delete rv_replay_listings;
    delete rv_zone_stream_ids;
    delete rv_zone_streams;
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
        RVReplayListing listing;
        listing.time = Net_ReplayListGetTime(response_handle, i);
        listing.angle_type = angle_type;
        listing.zone_id = zone_id;
        listing.idx = i;

        Net_ReplayListGetPlayerName(response_handle, i, listing.name, sizeof(listing.name));

        Format(listing_id, sizeof(listing_id), "%d;%d;%d", angle_type, zone_id, i);

        PushArrayString(rv_replay_listing_ids, listing_id);
        PushArrayArray(rv_replay_listings, listing);
    }
}

// A replay selected in the zone menu has been downloaded. Load the stream.
public void RV_ProcessDownloadedReplay(int response_handle)
{
    int zone_id = Net_ReplayDownloadGetZoneId(response_handle);
    int angle_type = Net_ReplayDownloadGetAngleType(response_handle);
    int replay_idx = Net_ReplayDownloadGetIndex(response_handle);

    char stream_id[RV_REPLAY_ID_SIZE];
    Format(stream_id, sizeof(stream_id), "%d;%d;%d", angle_type, zone_id, replay_idx);

    // TODO Need to create a stream here using Net_ReplayDownloadReadData, or probably just save to file and use RV_LoadFileStream.

#if 0
    int listing_idx = FindStringInArray(rv_replay_listing_ids, stream_id);

    PushArrayString(rv_zone_stream_ids, stream_id);
#endif
}
