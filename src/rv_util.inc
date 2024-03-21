#if defined UTIL_INCLUDED
#endinput
#endif
#define UTIL_INCLUDED

public void RV_CopyVector3(float source[3], float dest[3])
{
    dest[0] = source[0];
    dest[1] = source[1];
    dest[2] = source[2];
}

public void RV_CopyVector2(float source[2], float dest[2])
{
    dest[0] = source[0];
    dest[1] = source[1];
}

public void RV_MaybeCreateDir(char[] path)
{
    if (!DirExists(path))
    {
        CreateDirectory(path, 511); // All access.
    }
}
