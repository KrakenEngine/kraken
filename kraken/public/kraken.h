//
//  Kraken
//
//  Copyright 2018 Kearwood Gilbert. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without modification, are
//  permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this list of
//  conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//  of conditions and the following disclaimer in the documentation and/or other materials
//  provided with the distribution.
//
//  THIS SOFTWARE IS PROVIDED BY KEARWOOD GILBERT ''AS IS'' AND ANY EXPRESS OR IMPLIED
//  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KEARWOOD GILBERT OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  The views and conclusions contained in the software and documentation are those of the
//  authors and should not be interpreted as representing official policies, either expressed
//  or implied, of Kearwood Gilbert.
//

#ifndef KRAKEN_H
#define KRAKEN_H

#include "context.h"

#define KR_NULL_HANDLE 0

typedef enum {
  KR_SUCCESS = 0,
  KR_ERROR_NOT_INITIALIZED = 1,
  KR_ERROR_NOT_IMPLEMENTED = 2,
  KR_ERROR_OUT_OF_BOUNDS = 3,
  KR_ERROR_NOT_MAPPED = 4,
  KR_ERROR_INCORRECT_TYPE = 5,
  KR_ERROR_UNEXPECTED = 0x10000000,
  KR_RESULT_MAX_ENUM = 0x7FFFFFFF
} KrResult;

typedef enum {
  KR_STRUCTURE_TYPE_INITIALIZE      = 0,
  KR_STRUCTURE_TYPE_SHUTDOWN        = 1,
  KR_STRUCTURE_TYPE_LOAD_RESOURCE   = 0x00010000,
  KR_STRUCTURE_TYPE_UNLOAD_RESOURCE = 0x00010001,
  KR_STRUCTURE_TYPE_SAVE_RESOURCE   = 0x00010002,
  KR_STRUCTURE_TYPE_MAP_RESOURCE    = 0x00010003,
  KR_STRUCTURE_TYPE_UNMAP_RESOURCE  = 0x00010004,
  KR_STRUCTURE_TYPE_CREATE_BUNDLE   = 0x00010005,
  KR_STRUCTURE_TYPE_MOVE_TO_BUNDLE  = 0x00010006,

  KR_STRUCTURE_TYPE_MAX_ENUM        = 0x7FFFFFFF
} KrStructureType;

typedef int KrResourceMapIndex;

typedef struct {
  KrStructureType sType;
  size_t resourceMapSize;
} KrInitializeInfo;

typedef struct {
  KrStructureType sType;
  const char* pResourcePath;
  KrResourceMapIndex resourceHandle;
} KrLoadResourceInfo;

typedef struct {
  KrStructureType sType;
  KrResourceMapIndex resourceHandle;
} KrUnloadResourceInfo;

typedef struct {
  KrStructureType sType;
  const char* presourceName;
  KrResourceMapIndex resourceHandle;
} KrMapResourceInfo;

typedef struct {
  KrStructureType sType;
  KrResourceMapIndex resourceHandle;
} KrUnmapResourceInfo;

typedef struct {
  KrStructureType sType;
  const char* pResourcePath;
  KrResourceMapIndex resourceHandle;
} KrSaveResourceInfo;

typedef struct {
  KrStructureType sType;
  const char* pBundleName;
  KrResourceMapIndex resourceHandle;
} KrCreateBundleInfo;

typedef struct {
  KrStructureType sType;
  KrResourceMapIndex resourceHandle;
  KrResourceMapIndex bundleHandle;
} KrMoveToBundleInfo;

KrResult KrInitialize(const KrInitializeInfo* pInitializeInfo);
KrResult KrShutdown();
KrResult KrLoadResource(const KrLoadResourceInfo* pLoadResourceInfo);
KrResult KrUnloadResource(const KrUnloadResourceInfo* pUnloadResourceInfo);
KrResult KrSaveResource(const KrSaveResourceInfo* pSaveResourceInfo);
KrResult KrMapResource(const KrMapResourceInfo* pMapResourceInfo);
KrResult KrUnmapResource(const KrUnmapResourceInfo* pUnmapResourceInfo);
KrResult KrCreateBundle(const KrCreateBundleInfo* pCreateBundleInfo);
KrResult KrMoveToBundle(const KrMoveToBundleInfo* pMoveToBundleInfo);

#endif // KRAKEN_H
