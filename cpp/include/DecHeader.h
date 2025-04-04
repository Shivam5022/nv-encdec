#pragma once


#include "../include/NvDecoder.h"
#include "../include/NvCodecUtils.h"
#include "../include/FFmpegDemuxer.h"
#include "../include/ColorSpace.h"
#include "../include/AppDecUtils.h"
#include "../include/DecHeader.h"

enum OutputFormat { native = 0, bgra, bgra64 };
void DecodeProc(int cuDevice, const char *szMediaUri,
                 enum OutputFormat eOutputFormat, const char *szOutFilePath);

class DecodeProc {

public:
  DecodeProc(int cuDevice, const char *szMediaUri,
             enum OutputFormat eOutputFormat, const char *szOutFilePath);

  char *getNext();

private:
    FFmpegDemuxer demuxer;
    NvDecoder dec;
    CUcontext cuContext = NULL;

    Rect cropRect = {};
    Dim resizeDim = {};
    unsigned int opPoint = 0;
    bool bDispAllLayers = false;
    bool bExtractUserSEIMessage = false;
    unsigned int decsurf = 0;


    int nVideoBytes = 0, nFrameReturned = 0, nFrame = 0;
    uint8_t *pVideo = NULL, *pFrame;
    bool bDecodeOutSemiPlanar = false;
    CUstream cuStream = NULL;
};
