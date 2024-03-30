#include "net_priv.h"

char* Net_ReadFileAsString(const char* path)
{
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (h == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    char* ret = NULL;

    LARGE_INTEGER large;
    GetFileSizeEx(h, &large);

    if (large.HighPart == 0 && large.LowPart < INT32_MAX)
    {
        ret = (char*)malloc(large.LowPart + 1);

        ReadFile(h, ret, large.LowPart, NULL, NULL);

        ret[large.LowPart] = 0;
    }

    CloseHandle(h);

    return ret;
}

int Net_ToUTF16(const char* value, int value_length, wchar_t* buf, int buf_chars)
{
    int length = MultiByteToWideChar(CP_UTF8, 0, value, value_length, buf, buf_chars);

    if (length < buf_chars)
    {
        buf[length] = 0;
    }

    return length;
}
