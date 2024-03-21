#if defined LOAD_INCLUDED
#endinput
#endif
#define LOAD_INCLUDED

#define RV_MAGIC 0x02
#define RV_BOOKMARK_SIZE 524 // Unknown external constant of unknown data.

enum struct RvStreamInput
{
    int buttons;
    float position[3];
    float angles[2];
    int reserved;
    float velocity[3];
}

ArrayList RV_LoadFrameStream(char[] path)
{
    File file = OpenFile(path, "rb");

    if (file == null)
    {
        PrintToChatAll("Could not load stream \"%s\"", path);
        return null;
    }

    PrintToChatAll("Loaded stream \"%s\"", path);

    ArrayList ret = CreateArray(sizeof(RVFrame));

    int magic;
    ReadFileCell(file, magic, 4);

    if (magic != RV_MAGIC)
    {
        PrintToChatAll("Could not load stream \"%s\", wrong version or wrong file type", path);
        delete file;
        return null;
    }

    int dummy;
    ReadFileCell(file, dummy, 4); // Unknown data.

    int num_ticks;
    ReadFileCell(file, num_ticks, 4);

    int num_bookmarks;
    ReadFileCell(file, num_bookmarks, 4);

    int first_frame_pos = FilePosition(file) + num_bookmarks * RV_BOOKMARK_SIZE;

    ResizeArray(ret, num_ticks);

    FileSeek(file, first_frame_pos, SEEK_SET);

    for (int i = 0; i < num_ticks; i++)
    {
        RvStreamInput input;
        ReadFile(file, input, sizeof(input), 4);

        RVFrame output;
        output.buttons = input.buttons;

        RV_CopyVector3(input.position, output.position);
        RV_CopyVector2(input.angles, output.angles);
        RV_CopyVector3(input.velocity, output.velocity);

        SetArrayArray(ret, i, output);
    }

    delete file;

    return ret;
}
