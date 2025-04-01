#pragma once
#include <cuda.h>
#include <exception>
#include <stdint.h>
#include "NvDecoder/NvDecoder.h"
#include "NvEncoder/NvEncoderCuda.h"
#include "../Utils/NvEncoderCLIOptions.h"


void EncodeProc(CUdevice cuDevice, int nWidth, int nHeight, NV_ENC_BUFFER_FORMAT eFormat, NvEncoderInitParam *pEncodeCLIOptions,
    bool bBgra64, const char *szInFilePath, const char *szMediaPath, std::exception_ptr &encExceptionPtr);
