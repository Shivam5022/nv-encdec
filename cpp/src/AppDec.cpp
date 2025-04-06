/*
* Copyright 2017-2024 NVIDIA Corporation.  All rights reserved.
*
* Please refer to the NVIDIA end user license agreement (EULA) associated
* with this source code for terms and conditions that govern your use of
* this software. Any use, reproduction, disclosure, or distribution of
* this software and related documentation outside the terms of the EULA
* is strictly prohibited.
*
*/

/**
*  This sample application illustrates the encoding and streaming of a video
*  with one thread while another thread receives and decodes the video.
*  HDR video streaming is also demonstrated in this application.
*/

// #include <cuda.h>
#include <codecvt>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <memory>
#include <stdint.h>
#include "../include/NvDecoder.h"
#include "../include/NvCodecUtils.h"
#include "../include/FFmpegDemuxer.h"
#include "../include/ColorSpace.h"
#include "../include/AppDecUtils.h"
#include "../include/DecHeader.h"

simplelogger::Logger *logger = simplelogger::LoggerFactory::CreateConsoleLogger();

static std::vector<std::string> vstrOutputFormatName = 
{
    "native", "bgra", "bgra64"
};

void DecodeProc2(CUdevice cuDevice, const char *szMediaUri, OutputFormat eOutputFormat, const char *szOutFilePath)
{
    std::cout << "SV and Satyam are using GPU\n";
    std::ofstream fpOut(szOutFilePath, std::ios::out | std::ios::binary);
    if (!fpOut)
    {
        std::ostringstream err;
        err << "Unable to open output file: " << szOutFilePath << std::endl;
        throw std::invalid_argument(err.str());
    }
    CUstream cuStream = NULL;
    ck(cuInit(0));
    CUcontext cuContext = NULL;
    createCudaContext(&cuContext, 0, 0);
    FFmpegDemuxer demuxer(szMediaUri);

    Rect cropRect = {};
    Dim resizeDim = {};
    unsigned int opPoint = 0;
    bool bDispAllLayers = false;
    bool bExtractUserSEIMessage = false;
    unsigned int decsurf = 0;

    NvDecoder dec(cuContext, false, FFmpeg2NvCodecId(demuxer.GetVideoCodec()), false, false, &cropRect, &resizeDim, bExtractUserSEIMessage, 0, 0, 1000, false, decsurf, cuStream);

    /* Set operating point for AV1 SVC. It has no impact for other profiles or codecs
     * PFNVIDOPPOINTCALLBACK Callback from video parser will pick operating point set to NvDecoder  */
    // dec.SetOperatingPoint(opPoint, bDispAllLayers);

    int nVideoBytes = 0, nFrameReturned = 0, nFrame = 0;
    uint8_t *pVideo = NULL, *pFrame;
    bool bDecodeOutSemiPlanar = false;
    do {
        demuxer.Demux(&pVideo, &nVideoBytes);
        std::cout << nVideoBytes << " bytes are returned by demuxer\n";
        nFrameReturned = dec.Decode(pVideo, nVideoBytes);
        std::cout << nFrameReturned << std::endl;
        break;
        if (!nFrame && nFrameReturned)
            LOG(INFO) << dec.GetVideoInfo();
        
        // bDecodeOutSemiPlanar = (dec.GetOutputFormat() == cudaVideoSurfaceFormat_NV12) || (dec.GetOutputFormat() == cudaVideoSurfaceFormat_P016)
                               // || (dec.GetOutputFormat() == cudaVideoSurfaceFormat_NV16) || (dec.GetOutputFormat() == cudaVideoSurfaceFormat_P216);
        
        for (int i = 0; i < nFrameReturned; i++) {
            pFrame = dec.GetFrame();
            // if(bExtStream)
            // {   
            //     // If using external stream App needs to wait for memcpy to complete. 
            //     ck(cuStreamSynchronize(cuStream));          
            // }
            // if (bOutPlanar && bDecodeOutSemiPlanar) {
            //     ConvertSemiplanarToPlanar(pFrame, dec.GetWidth(), dec.GetHeight(), dec.GetBitDepth(), dec.GetOutputChromaFormat());
            // }
            // dump YUV to disk
            if (dec.GetWidth() == dec.GetDecodeWidth())
            {
                fpOut.write(reinterpret_cast<char*>(pFrame), dec.GetFrameSize());
            }
            else
            {
                // 4:2:0/4:2:2 output width is 2 byte aligned. If decoded width is odd , luma has 1 pixel padding
                // Remove padding from luma while dumping it to disk
                // dump luma
                for (auto i = 0; i < dec.GetHeight(); i++)
                {
                    fpOut.write(reinterpret_cast<char*>(pFrame), dec.GetDecodeWidth()*dec.GetBPP());
                    pFrame += dec.GetWidth() * dec.GetBPP();
                }
                // dump Chroma
                fpOut.write(reinterpret_cast<char*>(pFrame), dec.GetChromaPlaneSize());
            }
        }
        nFrame += nFrameReturned;
    } while (nVideoBytes);
    
    if(cuStream)
    {
        ck(cuStreamDestroy(cuStream));
        cuStream = NULL;
        CUcontext tempCuContext;
        ck(cuCtxPopCurrent(&tempCuContext));
    }

    std::vector <std::string> aszDecodeOutFormat = { "NV12", "P016", "YUV444", "YUV444P16", "NV16", "P216" };
    // if (bOutPlanar) {
    //     aszDecodeOutFormat[0] = "iyuv";     aszDecodeOutFormat[1] = "yuv420p16";
    //     aszDecodeOutFormat[4] = "yuv422p";  aszDecodeOutFormat[5] = "yuv422p16";
    // }
    std::cout << "Total frame decoded: " << nFrame << std::endl
            << "Saved in file " << szOutFilePath << " in "
            << aszDecodeOutFormat[dec.GetOutputFormat()]
            << " format" << std::endl;
    fpOut.close();
}

DecodeProc::DecodeProc(int cuDevice, const char *szMediaUri,
             enum OutputFormat eOutputFormat, const char *szOutFilePath) 
{
    std::cout << "Shivam and Satyam have fixed CPP code\n";
    CUstream cuStream = NULL;
    ck(cuInit(0));
    CUcontext cuContext = NULL;
    createCudaContext(&cuContext, 0, 0);
    demuxer = std::unique_ptr<FFmpegDemuxer>(new FFmpegDemuxer(szMediaUri));
    dec = std::unique_ptr<NvDecoder>(new NvDecoder(cuContext, false, FFmpeg2NvCodecId(demuxer->GetVideoCodec()), false, false, &cropRect, &resizeDim, bExtractUserSEIMessage, 0, 0, 1000, false, decsurf, cuStream));
}

char* DecodeProc::getNext() {
    while(1) {
        if (nFrameReturned == 0) {
            demuxer->Demux(&pVideo, &nVideoBytes);
            if (!nVideoBytes) {
                return nullptr;
            }
            nFrameReturned = dec->Decode(pVideo, nVideoBytes);
            nFrame += nFrameReturned;
        }
       
        if (nFrameReturned) {
            pFrame = dec->GetFrame();
            nFrameReturned--;
            return reinterpret_cast<char*>(pFrame);        
        }
    }

}

