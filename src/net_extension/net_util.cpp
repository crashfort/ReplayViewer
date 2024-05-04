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
        ret = (char*)Net_Alloc(large.LowPart + 1);

        ReadFile(h, ret, large.LowPart, NULL, NULL);

        ret[large.LowPart] = 0;
    }

    CloseHandle(h);

    return ret;
}

int32_t Net_ToUTF16(const char* value, int32_t value_length, wchar_t* buf, int32_t buf_chars)
{
    int32_t length = MultiByteToWideChar(CP_UTF8, 0, value, value_length, buf, buf_chars);

    if (length < buf_chars)
    {
        buf[length] = 0;
    }

    return length;
}

void* Net_Alloc(int32_t size)
{
    return malloc(size);
}

void* Net_ZAlloc(int32_t size)
{
    void* m = Net_Alloc(size);
    memset(m, 0, size);
    return m;
}

void Net_Free(void* addr)
{
    free(addr);
}

json_value_s* Net_FindJsonValueInObject(json_object_s* obj, const char* name)
{
    for (json_object_element_s* obj_elm = obj->start; obj_elm; obj_elm = obj_elm->next)
    {
        if (!strcmp(obj_elm->name->string, name))
        {
            return obj_elm->value;
        }
    }

    return NULL;
}

size_t Net_FindJsonValuesInObject(json_object_s* obj, NetJsonFindPair* find, size_t num)
{
    size_t found = 0;

    for (size_t i = 0; i < num; i++)
    {
        NetJsonFindPair* f = &find[i];

        *f->val = Net_FindJsonValueInObject(obj, f->name);

        if (*f->val)
        {
            found++;
        }
    }

    return found;
}

const char* Net_GetJsonString(json_value_s* val)
{
    assert(val->type == json_type_string);
    json_string_s* casted = json_value_as_string(val);
    return casted->string;
}

bool Net_GetJsonBool(json_value_s* val)
{
    if (val->type == json_type_true)
    {
        return true;
    }

    if (val->type == json_type_false)
    {
        return false;
    }

    assert(false);
}
