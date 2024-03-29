//
//  kraken.h
//  Kraken Engine
//  Copyright 2024 Kearwood Gilbert. All rights reserved.
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

#pragma once

#include "context.h"
#include "hydra.h"
#include <stdint.h>

#define KR_NULL_HANDLE 0

typedef enum
{
  KR_SUCCESS = 0,
  KR_ERROR_NOT_INITIALIZED,
  KR_ERROR_NOT_IMPLEMENTED,
  KR_ERROR_OUT_OF_BOUNDS,
  KR_ERROR_NOT_MAPPED,
  KR_ERROR_INCORRECT_TYPE,
  KR_ERROR_NOT_FOUND,
  KR_ERROR_AMBIGUOUS_MATCH,
  KR_ERROR_DUPLICATE_HANDLE,
  KR_ERROR_VULKAN,
  KR_ERROR_VULKAN_REQUIRED,
  KR_ERROR_VULKAN_SWAP_CHAIN,
  KR_ERROR_VULKAN_FRAMEBUFFER,
  KR_ERROR_VULKAN_DEPTHBUFFER,
  KR_ERROR_NO_DEVICE,
  KR_ERROR_SHADER_COMPILE_FAILED,
  KR_ERROR_UNEXPECTED = 0x10000000,
  KR_RESULT_MAX_ENUM = 0x7FFFFFFF
} KrResult;

typedef enum
{
  KR_STRUCTURE_TYPE_INITIALIZE = 0,
  KR_STRUCTURE_TYPE_SHUTDOWN,

  KR_STRUCTURE_TYPE_CREATE_WINDOW_SURFACE,
  KR_STRUCTURE_TYPE_DELETE_WINDOW_SURFACE,

  KR_STRUCTURE_TYPE_LOAD_RESOURCE = 0x00010000,
  KR_STRUCTURE_TYPE_UNLOAD_RESOURCE,
  KR_STRUCTURE_TYPE_GET_RESOURCE_DATA,
  KR_STRUCTURE_TYPE_SAVE_RESOURCE,
  KR_STRUCTURE_TYPE_MAP_RESOURCE,
  KR_STRUCTURE_TYPE_UNMAP_RESOURCE,
  KR_STRUCTURE_TYPE_CREATE_BUNDLE,
  KR_STRUCTURE_TYPE_MOVE_TO_BUNDLE,

  KR_STRUCTURE_TYPE_COMPILE_ALL_SHADERS,

  KR_STRUCTURE_TYPE_CREATE_SCENE = 0x00020000,

  KR_STRUCTURE_TYPE_FIND_NODE_BY_NAME = 0x00030000,
  KR_STRUCTURE_TYPE_FIND_ADJACENT_NODES,
  KR_STRUCTURE_TYPE_CREATE_NODE,
  KR_STRUCTURE_TYPE_DELETE_NODE,
  KR_STRUCTURE_TYPE_DELETE_NODE_CHILDREN,
  KR_STRUCTURE_TYPE_UPDATE_NODE,
  KR_STRUCTURE_TYPE_SET_NODE_LOCAL_TRANSFORM,
  KR_STRUCTURE_TYPE_SET_NODE_WORLD_TRANSFORM,

  KR_STRUCTURE_TYPE_NODE = 0x10000000,
  KR_STRUCTURE_TYPE_NODE_CAMERA,
  KR_STRUCTURE_TYPE_NODE_LOD_SET,
  KR_STRUCTURE_TYPE_NODE_LOD_GROUP,
  KR_STRUCTURE_TYPE_NODE_POINT_LIGHT,
  KR_STRUCTURE_TYPE_NODE_DIRECTIONAL_LIGHT,
  KR_STRUCTURE_TYPE_NODE_SPOT_LIGHT,
  KR_STRUCTURE_TYPE_NODE_SPRITE,
  KR_STRUCTURE_TYPE_NODE_MODEL,
  KR_STRUCTURE_TYPE_NODE_COLLIDER,
  KR_STRUCTURE_TYPE_NODE_BONE,
  KR_STRUCTURE_TYPE_NODE_LOCATOR,
  KR_STRUCTURE_TYPE_NODE_AUDIO_SOURCE,
  KR_STRUCTURE_TYPE_NODE_AMBIENT_ZONE,
  KR_STRUCTURE_TYPE_NODE_REVERB_ZONE,
  KR_STRUCTURE_TYPE_NODE_MAX_ENUM,

  KR_STRUCTURE_TYPE_MAX_ENUM = 0x7FFFFFFF
} KrStructureType;

typedef enum
{
  KR_SCENE_NODE_INSERT_BEFORE = 0,
  KR_SCENE_NODE_INSERT_AFTER,
  KR_SCENE_NODE_PREPEND_CHILD,
  KR_SCENE_NODE_APPEND_CHILD,
  KR_SCENE_NODE_INSERT_MAX_ENUM
} KrSceneNodeInsertLocation;

typedef int KrResourceMapIndex;
typedef int KrSceneNodeMapIndex;
typedef int KrSurfaceMapIndex;

typedef struct
{
  KrStructureType sType;
  size_t resourceMapSize;
  size_t nodeMapSize;
} KrInitializeInfo;

typedef struct
{
  KrStructureType sType;
  KrSurfaceMapIndex surfaceHandle;
  void* platformHandle; // Can static cast to HWND on Windows and CAMetalLayer* on macOS
} KrCreateWindowSurfaceInfo;

typedef struct
{
  KrStructureType sType;
  KrSurfaceMapIndex surfaceHandle;
} KrDeleteWindowSurfaceInfo;

typedef struct
{
  KrStructureType sType;
  const char* pResourcePath;
  KrResourceMapIndex resourceHandle;
} KrLoadResourceInfo;

typedef struct
{
  KrStructureType sType;
  KrResourceMapIndex resourceHandle;
} KrUnloadResourceInfo;

typedef struct
{
  KrStructureType sType;
  KrResourceMapIndex resourceHandle;
} KrGetResourceDataInfo;

typedef struct
{
  KrResult result;
  void* data;
  size_t length;
} KrGetResourceDataResult;

typedef void (*KrGetResourceDataCallback)(const KrGetResourceDataResult&);

typedef struct
{
  KrStructureType sType;
  const char* pResourceName;
  KrResourceMapIndex resourceHandle;
} KrMapResourceInfo;

typedef struct
{
  KrStructureType sType;
  KrResourceMapIndex resourceHandle;
} KrUnmapResourceInfo;

typedef struct
{
  KrStructureType sType;
  const char* pResourcePath;
  KrResourceMapIndex resourceHandle;
} KrSaveResourceInfo;

typedef struct
{
  KrStructureType sType;
  const char* pBundleName;
  KrResourceMapIndex resourceHandle;
} KrCreateBundleInfo;

typedef struct
{
  KrStructureType sType;
  KrResourceMapIndex resourceHandle;
  KrResourceMapIndex bundleHandle;
} KrMoveToBundleInfo;

typedef struct
{
  KrStructureType sType;
  KrResourceMapIndex bundleHandle;
  KrResourceMapIndex logHandle;
} KrCompileAllShadersInfo;

typedef struct
{
  KrStructureType sType;
  const char* pSceneName;
  KrResourceMapIndex resourceHandle;
} KrCreateSceneInfo;

typedef struct
{
  KrStructureType sType;
  const char* pName;
  hydra::Vector3 translate;
  hydra::Vector3 scale;
  hydra::Vector3 rotate;
  hydra::Vector3 pre_rotate;
  hydra::Vector3 post_rotate;
  hydra::Vector3 rotate_offset;
  hydra::Vector3 scale_offset;
  hydra::Vector3 rotate_pivot;
  hydra::Vector3 scale_pivot;
  union
  {
    struct
    {
      // KR_STRUCTURE_TYPE_NODE
      // No additional members
    } node;
    struct
    {
      // KR_STRUCTURE_TYPE_NODE_CAMERA
      KrSurfaceMapIndex surfaceHandle;
      KrResourceMapIndex skybox_texture;
    } camera;
    struct
    {
      // KR_STRUCTURE_TYPE_NODE_LOD_SET
      // No additional members
    } lod_set;
    struct
    {
      // KR_STRUCTURE_TYPE_NODE_LOD_GROUP
      float min_distance;
      float max_distance;
      hydra::Vector3 reference_min;
      hydra::Vector3 reference_max;
      bool use_world_units;
    } lod_group;
    struct
    {
      hydra::Vector3 color;
      float intensity;
      float decay_start;
      float flare_size;
      float flare_occlusion_size;
      KrResourceMapIndex flare_texture;
      bool casts_shadow;
      bool light_shafts;
      float dust_particle_density;
      float dust_particle_size;
      float dust_particle_intensity;
      struct
      {
        // KR_STRUCTURE_TYPE_NODE_POINT_LIGHT
        // No additional members
      } point;
      struct
      {
        // KR_STRUCTURE_TYPE_NODE_DIRECTIONAL_LIGHT
        // No additional members
      } directional;
      struct
      {
        // KR_STRUCTURE_TYPE_NODE_SPOT_LIGHT
        float inner_angle;
        float outer_angle;
      } spot;
    } light;
    struct
    {
      // KR_STRUCTURE_TYPE_NODE_SPRITE
      KrResourceMapIndex texture;
      float alpha;
    } sprite;
    struct
    {
      // KR_STRUCTURE_TYPE_NODE_MODEL
      float lod_min_coverage;
      bool receives_shadow;
      bool faces_camera;
      float rim_power;
      hydra::Vector3 rim_color;
      KrResourceMapIndex mesh;
      KrResourceMapIndex light_map_texture;
    } model;
    struct
    {
      // KR_STRUCTURE_TYPE_NODE_COLLIDER
      KrResourceMapIndex mesh;
      uint64_t layer_mask;
      float audio_occlusion;
    } collider;
    struct
    {
      // KR_STRUCTURE_TYPE_NODE_BONE
      // No additional members
    } bone;
    struct
    {
      // KR_STRUCTURE_TYPE_NODE_LOCATOR
      // No additional members
    } locator;
    struct
    {
      // KR_STRUCTURE_TYPE_NODE_AUDIO_SOURCE
      KrResourceMapIndex sample;
      float gain;
      float pitch;
      bool looping;
      bool is_3d;
      float reference_distance;
      float reverb;
      float rolloff_factor;
      bool enable_obstruction;
      bool enable_occlusion;
    } audio_source;
    struct
    {
      // KR_STRUCTURE_TYPE_NODE_AMBIENT_ZONE
      char* pZoneName;
      float gradient;
      float gain;
      KrResourceMapIndex sample;
    } ambient_zone;
    struct
    {
      // KR_STRUCTURE_TYPE_NODE_REVERB_ZONE
      char* pZoneName;
      float gradient;
      float gain;
      KrResourceMapIndex sample;
    } reverb_zone;
  };
} KrNodeInfo;

typedef struct
{
  KrStructureType sType;
  KrResourceMapIndex sceneHandle;
  KrSceneNodeMapIndex nodeHandle;
  char* pName;
} KrFindNodeByNameInfo;

typedef struct
{
  KrStructureType sType;
  KrResourceMapIndex sceneHandle;
  KrSceneNodeMapIndex nodeHandle;
  KrSceneNodeMapIndex parentNodeHandle;
  KrSceneNodeMapIndex priorNodeHandle;
  KrSceneNodeMapIndex nextNodeHandle;
  KrSceneNodeMapIndex firstChildNodeHandle;
  KrSceneNodeMapIndex lastChildNodeHandle;
} KrFindAdjacentNodesInfo;

typedef struct
{
  KrStructureType sType;
  KrResourceMapIndex sceneHandle;
  KrSceneNodeMapIndex nodeHandle;
} KrDeleteNodeInfo;

typedef struct
{
  KrStructureType sType;
  KrResourceMapIndex sceneHandle;
  KrSceneNodeMapIndex nodeHandle;
} KrDeleteNodeChildrenInfo;

typedef struct
{
  KrStructureType sType;
  KrResourceMapIndex sceneHandle;
  KrSceneNodeMapIndex newNodeHandle;
  KrSceneNodeMapIndex relativeNodeHandle;
  KrSceneNodeInsertLocation location;
  KrNodeInfo node;
} KrCreateNodeInfo;

typedef struct
{
  KrStructureType sType;
  KrResourceMapIndex sceneHandle;
  KrSceneNodeMapIndex nodeHandle;
  KrNodeInfo node;
} KrUpdateNodeInfo;

typedef struct
{
  KrStructureType sType;
  KrResourceMapIndex sceneHandle;
  KrSceneNodeMapIndex nodeHandle;
  hydra::Vector3 translate;
  hydra::Vector3 scale;
  hydra::Vector3 rotate;
} KrSetNodeLocalTransformInfo;

typedef struct
{
  KrStructureType sType;
  KrResourceMapIndex sceneHandle;
  KrSceneNodeMapIndex nodeHandle;
  hydra::Vector3 translate;
  hydra::Vector3 scale;
  hydra::Vector3 rotate;
} KrSetNodeWorldTransformInfo;

KrResult KrInitialize(const KrInitializeInfo* pInitializeInfo);
KrResult KrShutdown();
KrResult KrCreateWindowSurface(const KrCreateWindowSurfaceInfo* pCreateWindowSurfaceInfo);
KrResult KrDeleteWindowSurface(const KrDeleteWindowSurfaceInfo* pDeleteWindowSurfaceInfo);

KrResult KrLoadResource(const KrLoadResourceInfo* pLoadResourceInfo);
KrResult KrUnloadResource(const KrUnloadResourceInfo* pUnloadResourceInfo);
KrResult KrGetResourceData(const KrGetResourceDataInfo* pGetResourceDataInfo, KrGetResourceDataCallback callback);
KrResult KrSaveResource(const KrSaveResourceInfo* pSaveResourceInfo);
KrResult KrMapResource(const KrMapResourceInfo* pMapResourceInfo);
KrResult KrUnmapResource(const KrUnmapResourceInfo* pUnmapResourceInfo);
KrResult KrCreateBundle(const KrCreateBundleInfo* pCreateBundleInfo);
KrResult KrMoveToBundle(const KrMoveToBundleInfo* pMoveToBundleInfo);
KrResult KrInitNodeInfo(KrNodeInfo* pNodeInfo, KrStructureType nodeType);

KrResult KrCompileAllShaders(const KrCompileAllShadersInfo* pCompileAllShadersInfo);

KrResult KrCreateScene(const KrCreateSceneInfo* pCreateSceneInfo);
KrResult KrFindNodeByName(const KrFindNodeByNameInfo* pFindNodeByNameInfo);
KrResult KrFindAdjacentNodes(const KrFindAdjacentNodesInfo* pFindAdjacentNodesInfo);
KrResult KrSetNodeLocalTransform(const KrSetNodeLocalTransformInfo* pSetNodeLocalTransform);
KrResult KrSetNodeWorldTransform(const KrSetNodeWorldTransformInfo* pSetNodeWorldTransform);
KrResult KrCreateNode(const KrCreateNodeInfo* pCreateNodeInfo);
KrResult KrUpdateNode(const KrUpdateNodeInfo* pUpdateNodeInfo);
KrResult KrDeleteNode(const KrDeleteNodeInfo* pDeleteNodeInfo);
KrResult KrDeleteNodeChildren(const KrDeleteNodeChildrenInfo* pDeleteNodeChildrenInfo);
