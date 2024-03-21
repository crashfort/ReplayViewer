#if defined LOAD_INCLUDED
#endinput
#endif
#define LOAD_INCLUDED

#define RV_MAGIC 0x02
#define RV_BOOKMARK_SIZE 524 // Unknown external constant of unknown data.

// Data that we read from the file.
enum struct RvStreamInput
{
    int buttons;
    float position[3];
    float angles[2];
    int reserved;
    float velocity[3];
}

bool RV_LoadFrameStream(char[] path, RVStream stream)
{
    File file = OpenFile(path, "rb");

    if (file == null)
    {
        PrintToChatAll("Could not load stream \"%s\"", path);
        return false;
    }

    PrintToChatAll("Loaded stream \"%s\"", path);

    int magic;
    ReadFileCell(file, magic, 4);

    if (magic != RV_MAGIC)
    {
        PrintToChatAll("Could not load stream \"%s\", wrong version or wrong file type", path);
        delete file;
        return false;
    }

    int dummy;
    ReadFileCell(file, dummy, 4); // Unknown data.

    int num_frames;
    ReadFileCell(file, num_frames, 4);

    int num_bookmarks;
    ReadFileCell(file, num_bookmarks, 4);

    int first_frame_pos = FilePosition(file) + num_bookmarks * RV_BOOKMARK_SIZE;

    int start_offset;
    ReadFileCell(file, start_offset, 4);

    stream.frames = CreateArray(sizeof(RVFrame));
    ResizeArray(stream.frames, num_frames);

    FileSeek(file, first_frame_pos, SEEK_SET);

    for (int i = 0; i < num_frames; i++)
    {
        RvStreamInput input;
        ReadFile(file, input, sizeof(input), 4);

        RVFrame output;
        output.buttons = input.buttons;

        RV_CopyVector3(input.position, output.position);
        RV_CopyVector2(input.angles, output.angles);
        RV_CopyVector3(input.velocity, output.velocity);

        SetArrayArray(stream.frames, i, output);
    }

    stream.num_frames = num_frames;
    stream.start_offset = start_offset;

    delete file;

    return true;
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

    RV_HaveBotsPlaying = true;
}

public void RV_FreeStream(RVStream stream)
{
    if (stream.frames != null)
    {
        delete stream.frames;
        stream.frames = null;
    }
}

public void RV_DupeStream(RVStream source, RVStream dest)
{
    dest.num_frames = source.num_frames;
    dest.start_offset = source.start_offset;
    dest.frames = RV_CloneArrayHandle(source.frames);
}
