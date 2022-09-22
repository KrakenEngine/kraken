//
//  hello_cube.cpp
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
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
#include "hello_cube.h"
#include "kraken.h"
#include <assert.h>

void smoke_load()
{
  enum
  {
    kSceneResourceHandle = 10,
    kSkyboxTextureResourceHandle
  };

  enum
  {
    kCameraNodeHandle = 10
  };

  /*
  KrMapResourceInfo map_skybox_resource = { KR_STRUCTURE_TYPE_MAP_RESOURCE };
  map_skybox_resource.pResourceName = "skybox";
  map_skybox_resource.resourceHandle = kSkyboxTextureResourceHandle;
  KrResult res = KrMapResource(&map_skybox_resource);
  assert(res == KR_SUCCESS);
  */

  // Create a scene
  KrCreateSceneInfo create_scene_info = { KR_STRUCTURE_TYPE_CREATE_SCENE };
  create_scene_info.resourceHandle = kSceneResourceHandle;
  create_scene_info.pSceneName = "my_scene";
  KrResult res = KrCreateScene(&create_scene_info);
  assert(res == KR_SUCCESS);

  /*
  // Add a camera to the scene
  KrCreateNodeInfo create_camera_info = { KR_STRUCTURE_TYPE_CREATE_NODE };
  res = KrInitNodeInfo(&create_camera_info.node, KR_STRUCTURE_TYPE_NODE_CAMERA);
  assert(res == KR_SUCCESS);
  create_camera_info.relativeNodeHandle = -1;
  create_camera_info.newNodeHandle = kCameraNodeHandle;
  create_camera_info.sceneHandle = kSceneResourceHandle;
  create_camera_info.node.pName = "my_camera";
  create_camera_info.node.camera.surface = 1;
  // create_camera_info.node.camera.skybox_texture = kSkyboxTextureResourceHandle;
  res = KrCreateNode(&create_camera_info);
  assert(res == KR_SUCCESS);
  */
}
