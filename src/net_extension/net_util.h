#pragma once

char* Net_ReadFileAsString(const char* path);

int Net_ToUTF16(const char* value, int value_length, wchar_t* buf, int buf_chars);
