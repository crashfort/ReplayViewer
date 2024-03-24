#if defined LOAD_INCLUDED
#endinput
#endif
#define LOAD_INCLUDED

#define RV_MAGIC 0x02 // Identifier or version.

bool RV_LoadFrameStream(char[] path, RVStream stream)
{
    RVFileMeta meta;
    RV_ParseFileMeta(path, meta);

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
public void RV_AddAllReplaysForMap(int client)
{
    char dir_path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, dir_path, sizeof(dir_path), "%s/replays/%s", RV_ROOT_PATH, rv_cur_map);

    if (!DirExists(dir_path))
    {
        PrintToChat(client, "No replays exist for map %s", rv_cur_map);
        return;
    }

    DirectoryListing dir_lister = OpenDirectory(dir_path);

    if (dir_lister == null)
    {
        PrintToChat(client, "Could not open directory for map %s", rv_cur_map);
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

        RV_SpawnBotWithStream(full_path);

        index++;
    }

    delete dir_lister;

    if (index == 0)
    {
        PrintToChat(client, "No replays available for %s", rv_cur_map);
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
    dest.start_offset = source.start_offset;
    dest.num_frames = source.num_frames;
    dest.num_bookmarks = source.num_bookmarks;
    dest.frames = RV_CloneArrayHandle(source.frames);
    dest.bookmarks = RV_CloneArrayHandle(source.bookmarks);
}

// Extract additional information from the file name if we can.
public bool RV_ParseFileMeta(char[] name, RVFileMeta meta)
{
    int idx = StrContains(name, RV_FILE_PREFIX);

    if (idx == -1)
    {
        return false;
    }

    idx += strlen(RV_FILE_PREFIX);

    char tokens[4][32];
    int num_read = ExplodeString(name[idx], "_", tokens, sizeof(tokens), sizeof(tokens[]));

    if (num_read != 4)
    {
        return false;
    }

    meta.map_id = StringToInt(tokens[0]);
    meta.mode = StringToInt(tokens[1]);
    meta.player_id = StringToInt(tokens[2]);
    StringToInt64(tokens[3], meta.timestamp);

    // PrintToServer("map_id=%d", meta.map_id);
    // PrintToServer("mode=%d", meta.mode);
    // PrintToServer("player_id=%d", meta.player_id);
    // PrintToServer("time_low=%d", meta.timestamp[0]);
    // PrintToServer("time_high=%d", meta.timestamp[1]);

    return true;
}
