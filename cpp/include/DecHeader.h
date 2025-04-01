#pragma once

enum OutputFormat
{
    native = 0, bgra, bgra64
};
void DecodeProc(int cuDevice, const char *szMediaUri, enum OutputFormat eOutputFormat, const char *szOutFilePath);

