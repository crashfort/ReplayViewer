#if defined UTIL_INCLUDED
#endinput
#endif
#define UTIL_INCLUDED

#define RV_ZERO_VECTOR3 { 0.0, 0.0, 0.0 }

public void RV_CopyVector3(float source[3], float dest[3])
{
    dest[0] = source[0];
    dest[1] = source[1];
    dest[2] = source[2];
}

// Copies pitch and yaw, ignores roll.
public void RV_CopyAngles(float source[3], float dest[3])
{
    dest[0] = source[0];
    dest[1] = source[1];
    dest[2] = 0.0;
}

public void RV_MaybeCreateDir(char[] path)
{
    if (!DirExists(path))
    {
        CreateDirectory(path, 511); // All access.
    }
}

public ArrayList RV_CloneArrayHandle(ArrayList source)
{
    return view_as<ArrayList>(CloneHandle(view_as<Handle>(source)));
}

public void RV_FormatTime(char[] buffer, int buffer_size, int ms)
{
    int minutes = (ms / 1000) / 60;
    int seconds = (ms / 1000) % 60;
    int milli = ms % 1000;

    FormatEx(buffer, buffer_size, "%02d:%02d.%03d", minutes, seconds, milli);
}

// Get time in milliseconds from number of frames.
public int RV_FramesToTime(int frames, float frame_time)
{
    int frame_time_reciprocal = RoundFloat(1.0 / frame_time);

    int source_tb_den = frame_time_reciprocal;
    int dest_tb_den = 1000;

    int ret = frames * dest_tb_den / source_tb_den;

    return ret;
}
