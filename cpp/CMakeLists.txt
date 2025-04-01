cmake_minimum_required(VERSION 3.7)

project(NvEncDec)

if(CMAKE_VERSION VERSION_GREATER "3.27.0" OR CMAKE_VERSION VERSION_EQUAL "3.27.0")   
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/External/cmake/Modules)
    message("Setting CMAKE_MODULE_PATH to ${CMAKE_MODULE_PATH}")
endif()

# Set C++11 for all projects and disable non-standard extensions
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_PREFIX $ENV{HOME}/.local)


# Check for WSL 
if (EXISTS /usr/lib/wsl/lib)
    set(WSL_FLAG TRUE)
endif()

set(NVCODEC_PUBLIC_INTERFACE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nvcodec_interface)
set(INSTALL_DIR lib)

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        Set(CUVID_LIB ${CMAKE_CURRENT_SOURCE_DIR}/../Lib/x64/nvcuvid.lib)
        set(NVENCODEAPI_LIB ${CMAKE_CURRENT_SOURCE_DIR}/../Lib/x64/nvencodeapi.lib)
    else()
        Set(CUVID_LIB ${CMAKE_CURRENT_SOURCE_DIR}/../Lib/Win32/nvcuvid.lib)
        set(NVENCODEAPI_LIB ${CMAKE_CURRENT_SOURCE_DIR}/../Lib/Win32/nvencodeapi.lib)
    endif()
else()
    find_library(CUVID_LIB nvcuvid)
    find_library(NVENCODEAPI_LIB nvidia-encode)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PC_AVCODEC REQUIRED IMPORTED_TARGET libavcodec)
    pkg_check_modules(PC_AVFORMAT REQUIRED IMPORTED_TARGET libavformat)
    pkg_check_modules(PC_AVUTIL REQUIRED IMPORTED_TARGET libavutil)
    pkg_check_modules(PC_SWRESAMPLE REQUIRED IMPORTED_TARGET libswresample)

    set(NV_FFMPEG_HDRS ${PC_AVCODEC_INCLUDE_DIRS})
    find_library(AVCODEC_LIBRARY NAMES avcodec
            HINTS
            ${PC_AVCODEC_LIBDIR}
            ${PC_AVCODEC_LIBRARY_DIRS}
            )
    find_library(AVFORMAT_LIBRARY NAMES avformat
            HINTS
            ${PC_AVFORMAT_LIBDIR}
            ${PC_AVFORMAT_LIBRARY_DIRS}
            )
    find_library(AVUTIL_LIBRARY NAMES avutil
            HINTS
            ${PC_AVUTIL_LIBDIR}
            ${PC_AVUTIL_LIBRARY_DIRS}
            )
    find_library(SWRESAMPLE_LIBRARY NAMES swresample
            HINTS
            ${PC_SWRESAMPLE_LIBDIR}
            ${PC_SWRESAMPLE_LIBRARY_DIRS}
            )
    set(AVCODEC_LIB ${AVCODEC_LIBRARY})
    set(AVFORMAT_LIB ${AVFORMAT_LIBRARY})
    set(AVUTIL_LIB ${AVUTIL_LIBRARY})
    set(SWRESAMPLE_LIB ${SWRESAMPLE_LIBRARY})
endif()

if(DEFINED FFMPEG_DIR)
    Set(FFMPEG_DLL_DIR ${FFMPEG_DIR}/bin/)
    Set(FFMPEG_LIB_DIR ${FFMPEG_DIR}/lib/)
endif()

if(DEFINED FFMPEG_DIR)
    file(GLOB externalLibList 
        ${FFMPEG_DLL_DIR}*.dll 
        ${FFMPEG_LIB_DIR}*.lib
    )    
    add_custom_target(copyFFMPEGFiles ALL)
    add_custom_command(TARGET copyFFMPEGFiles COMMAND ${CMAKE_COMMAND} -E make_directory  ${NVCODEC_SAMPLES_INSTALL_DIR}/$<CONFIG>/)

    foreach(externalLib ${externalLibList})
        add_custom_command(TARGET copyFFMPEGFiles COMMAND ${CMAKE_COMMAND} -E copy ${externalLib} ${NVCODEC_SAMPLES_INSTALL_DIR}/$<CONFIG>/)
    endforeach()
endif()


# Define the sources for the application
# set(APP_SOURCES
#     ${CMAKE_CURRENT_SOURCE_DIR}/AppEnc.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/AppDec.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp  # this is for the executable, will be excluded from the library
# )

# Define sources for the library (without main.cpp)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Print the sources to the console
# message(STATUS "Sources: ${SOURCES}")


# set(NV_ENC_SOURCES
#     ${NV_ENC_DIR}/NvEncoder.cpp
#     ${NV_ENC_DIR}/NvEncoderCuda.cpp
#     ${NV_DEC_DIR}/NvDecoder.cpp
# )

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/)
# message(STATUS "HEADERS: ${HEADERS}")

# set(NV_ENC_CUDA_UTILS
#     ${NVCODEC_UTILS_DIR}/ColorSpace.cu
# )

source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

find_package(CUDA)

set(CUDA_HOST_COMPILER ${CMAKE_CXX_COMPILER})

set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_50,code="sm_50,compute_50")
if (WIN32 AND CUDA_VERSION_MAJOR GREATER_EQUAL 11 AND CUDA_VERSION_MINOR GREATER_EQUAL 7)
    set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-Xcompiler /MT)
endif()

if (CMAKE_COMPILER_IS_GNUCC)
    if(NOT "${CUDA_NVCC_FLAGS}" MATCHES "-std=c\\+\\+11")
        list(APPEND CUDA_NVCC_FLAGS -std=c++11)
    endif()
endif()

if(WIN32)
    set(NV_FFMPEG_HDRS ${FFMPEG_DIR}/include)
endif()

# Create the library target without main.cpp (for static or shared library)
# cuda_add_executable(${PROJECT_NAME}  ${APP_SOURCES} ${NV_ENC_SOURCES} ${NV_ENC_CUDA_UTILS} ${NV_ENC_HDRS} ${NV_DEC_HDRS} ${NV_FFMPEG_HDRS})
cuda_add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# CUDA settings
set_target_properties(${PROJECT_NAME} PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${CUDA_INCLUDE_DIRS}
    ${NVCODEC_PUBLIC_INTERFACE_DIR}
    ${NV_FFMPEG_HDRS}
    ${HEADERS}
)

if(WIN32)
    Set(AVCODEC_LIB ${FFMPEG_DIR}/lib/avcodec.lib)
    Set(AVFORMAT_LIB ${FFMPEG_DIR}/lib/avformat.lib)
    Set(AVUTIL_LIB ${FFMPEG_DIR}/lib/avutil.lib)
    Set(SWRESAMPLE_LIB ${FFMPEG_DIR}/lib/swresample.lib)
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} ${CUDA_CUDA_LIBRARY} ${CMAKE_DL_LIBS} ${NVENCODEAPI_LIB} ${CUVID_LIB}
    ${AVCODEC_LIB} ${AVFORMAT_LIB} ${AVUTIL_LIB} ${SWRESAMPLE_LIB})

# Install library
install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_DIR})

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set_target_properties(AppEncDecLib PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${NVCODEC_SAMPLES_INSTALL_DIR}/$<CONFIG>/)
endif()

# Copyright 2020-2024 NVIDIA Corporation.  All rights reserved.
#
# Please refer to the NVIDIA end user license agreement (EULA) associated
# with this source code for terms and conditions that govern your use of
# this software. Any use, reproduction, disclosure, or distribution of
# this software and related documentation outside the terms of the EULA
# is strictly prohibited.

# # 3.7 is required for FindVulkan module support in CMake.
# cmake_minimum_required(VERSION 3.7)

# project(AppEncDec)

# set(APP_SOURCES
#    ${CMAKE_CURRENT_SOURCE_DIR}/AppEnc.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/AppDec.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp  # this is for the executable, will be excluded from the library
# )

# set(LIB_SOURCES
#     ${CMAKE_CURRENT_SOURCE_DIR}/AppEnc.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/AppDec.cpp
# )

# set(NV_ENC_SOURCES
#  ${NV_ENC_DIR}/NvEncoder.cpp
#  ${NV_ENC_DIR}/NvEncoderCuda.cpp
#  ${NV_DEC_DIR}/NvDecoder.cpp
#  )

# set(NV_ENC_HDRS
#  ${NV_ENC_DIR}/NvEncoder.h
#  ${NV_ENC_DIR}/NvEncoderCuda.h
#  ${NV_DEC_DIR}/NvDecoder.h
#  ${NVCODEC_PUBLIC_INTERFACE_DIR}/nvEncodeAPI.h
#  ${NVCODEC_UTILS_DIR}/NvCodecUtils.h
#  ${NVCODEC_UTILS_DIR}/NvEncoderCLIOptions.h
#  ${NVCODEC_UTILS_DIR}/ColorSpace.h
#  ${NVCODEC_UTILS_DIR}/FFmpegStreamer.h
#  ${NVCODEC_UTILS_DIR}/FFmpegDemuxer.h
#  ${NV_APPDEC_COMMON_DIR}/AppDecUtils.h
# )

# set(NV_ENC_CUDA_UTILS
#  ${NVCODEC_UTILS_DIR}/ColorSpace.cu
# )

# source_group( "headers" FILES ${NV_ENC_HDRS} )
# # source_group( "sources" FILES ${APP_SOURCES} ${NV_ENC_SOURCES} ${NV_ENC_CUDA_UTILS})
# source_group("sources" FILES ${APP_SOURCES} ${LIB_SOURCES} ${NV_ENC_SOURCES} ${NV_ENC_CUDA_UTILS})
# cuda_add_library(AppEncDecLib SHARED ${LIB_SOURCES} ${NV_ENC_SOURCES} ${NV_ENC_CUDA_UTILS} ${NV_ENC_HDRS} ${NV_DEC_HDRS} ${NV_FFMPEG_HDRS})

# find_package(CUDA)

# set(CUDA_HOST_COMPILER ${CMAKE_CXX_COMPILER})

# set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_50,code=\"sm_50,compute_50\")
# # CUDA 11.7 and later is compiled with /MT option (staticaly linked with C runtime), 
# # since we are statically linking cudart_static.lib with the app, 
# # we need to make sure that all .obj files linked are compiled with /MT to avoid linker warning LNK4098
# if (WIN32 AND CUDA_VERSION_MAJOR GREATER_EQUAL 11 AND CUDA_VERSION_MINOR GREATER_EQUAL 7)
#     set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-Xcompiler /MT)
# endif()
# if ( CMAKE_COMPILER_IS_GNUCC )
#     if(NOT "${CUDA_NVCC_FLAGS}" MATCHES "-std=c\\+\\+11" )
#         list(APPEND CUDA_NVCC_FLAGS -std=c++11)
#     endif()
# endif()

# if(WIN32)
#     set(NV_FFMPEG_HDRS ${FFMPEG_DIR}/include)
# endif()

# # cuda_add_executable(${PROJECT_NAME}  ${APP_SOURCES} ${NV_ENC_SOURCES} ${NV_ENC_CUDA_UTILS} ${NV_ENC_HDRS} ${NV_DEC_HDRS} ${NV_FFMPEG_HDRS})

# if (WIN32 AND CUDA_VERSION_MAJOR GREATER_EQUAL 11 AND CUDA_VERSION_MINOR GREATER_EQUAL 7)
#     target_compile_options(${PROJECT_NAME} PRIVATE "/MT")
# endif()
# set_target_properties(${PROJECT_NAME} PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

# target_include_directories(${PROJECT_NAME} PUBLIC ${CUDA_INCLUDE_DIRS}
#  ${NVCODEC_PUBLIC_INTERFACE_DIR}
#  ${NVCODEC_UTILS_DIR}
#  ${NV_FFMPEG_HDRS}
#  ${NV_CODEC_DIR}
# )

# if(WIN32)
#     Set(AVCODEC_LIB ${FFMPEG_DIR}/lib/avcodec.lib)
#     Set(AVFORMAT_LIB ${FFMPEG_DIR}/lib/avformat.lib)
#     Set(AVUTIL_LIB ${FFMPEG_DIR}/lib/avutil.lib)
#     Set(SWRESAMPLE_LIB ${FFMPEG_DIR}/lib/swresample.lib)
#     #copy dll's
#     install(DIRECTORY ${FFMPEG_DLL_DIR} DESTINATION ${NVCODEC_SAMPLES_INSTALL_DIR} OPTIONAL FILES_MATCHING PATTERN "*.dll")
#     install(DIRECTORY ${FFMPEG_LIB_DIR} DESTINATION ${NVCODEC_SAMPLES_INSTALL_DIR} OPTIONAL FILES_MATCHING PATTERN "*.lib")
# endif()

# target_link_libraries(${PROJECT_NAME} ${CUDA_CUDA_LIBRARY} ${CMAKE_DL_LIBS} ${NVENCODEAPI_LIB} ${CUVID_LIB} ${AVCODEC_LIB}
#  ${AVFORMAT_LIB} ${AVUTIL_LIB} ${SWRESAMPLE_LIB})

# install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${NVCODEC_SAMPLES_INSTALL_DIR})
# if (MSVC)
#     add_definitions(-D_CRT_SECURE_NO_WARNINGS)
#     set_target_properties( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${NVCODEC_SAMPLES_INSTALL_DIR}/$<CONFIG>/ )
#     add_dependencies(${PROJECT_NAME} copyFFMPEGFiles)
#     #copy pdb's
#     install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ${NVCODEC_SAMPLES_INSTALL_DIR}  CONFIGURATIONS Debug)
# endif()

