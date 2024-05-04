#pragma once

char* Net_ReadFileAsString(const char* path);

int32_t Net_ToUTF16(const char* value, int32_t value_length, wchar_t* buf, int32_t buf_chars);

#define NET_TO_UTF16(VALUE, DEST) Net_ToUTF16(VALUE, strlen(VALUE), DEST, NET_ARRAY_SIZE(DEST));

void* Net_Alloc(int32_t size);
void* Net_ZAlloc(int32_t size); // Zero init alloc.
void Net_Free(void* addr);

// Easier to type when you need to allocate structures.
#define NET_ZALLOC(T) (T*)Net_ZAlloc(sizeof(T))
#define NET_ZALLOC_NUM(T, NUM) (T*)Net_ZAlloc(sizeof(T) * NUM)

inline bool Net_IdxInRange(int32_t idx, int32_t size)
{
    return idx >= 0 && idx < size;
}

struct NetJsonFindPair
{
    const char* name;
    json_value_s** val;
};

json_value_s* Net_FindJsonValueInObject(json_object_s* obj, const char* name);
size_t Net_FindJsonValuesInObject(json_object_s* obj, NetJsonFindPair* find, size_t num);
const char* Net_GetJsonString(json_value_s* val);
bool Net_GetJsonBool(json_value_s* val);
