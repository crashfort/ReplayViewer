#if defined LOAD_INCLUDED
#endinput
#endif
#define LOAD_INCLUDED

// Possible versions.
#define RV_VERSION_2 2
#define RV_VERSION_3 3

// Version 3 has a different size RVFrame structure with more data, additionally it has extra block of additional metadata after the header.
// 10 cells here are referring to the definition we have of RVFrame.
#define RV_VERSION_2_NUM_FRAME_CELLS 10
#define RV_MAX_FRAME_CELLS 64

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

    if (header.magic != RV_VERSION_2 && header.magic != RV_VERSION_3)
    {
        PrintToChatAll("Could not load stream \"%s\", wrong version or wrong file type (%d)", path, header.magic);
        delete file;
        return false;
    }

    // In version 3 the size of the RVFrame structure is more dynamic, so read what size the file has.
    int frame_cells = RV_VERSION_2_NUM_FRAME_CELLS;

    if (header.magic == RV_VERSION_3)
    {
        // We always need to read this because it is only a smaller part of this versions frame structure that we actually care about.
        ReadFileCell(file, frame_cells, 4);

        int ext_cells;
        ReadFileCell(file, ext_cells, 4);

        if ((frame_cells < RV_VERSION_2_NUM_FRAME_CELLS) || (frame_cells > RV_MAX_FRAME_CELLS))
        {
            PrintToChatAll("Could not load stream \"%s\", unsupported frame size (%d cells)", path, frame_cells);
            delete file;
            return false;
        }

        // Ignore extra metadata after the header.
        if (ext_cells > 0)
        {
            FileSeek(file, ext_cells * 4, SEEK_CUR);
        }
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

    // Intermediate storage for the whole frame.
    int frame_scratch[RV_MAX_FRAME_CELLS];

    for (int i = 0; i < header.num_frames; i++)
    {
        // Read the whole frame as present in the file, and then discard everything larger than the frame size we care about.
        ReadFile(file, frame_scratch, frame_cells, 4);
        SetArrayArray(stream.frames, i, frame_scratch, sizeof(RVFrame));
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

public bool RV_LoadLocalFile(char[] file_name)
{
    char full_path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, full_path, sizeof(full_path), "%s/replays/%s/%s", RV_ROOT_PATH, rv_cur_map, file_name);

    RVStream stream;

    if (!RV_LoadFileStream(full_path, stream))
    {
        return false;
    }

    if (!RV_SpawnBotWithStream(stream, "crashfort/ReplayViewer"))
    {
        return false;
    }

    return true;
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
}

// Called on map end.
public void RV_StopLoad()
{
}
