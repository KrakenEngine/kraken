//
//  hello_cube.cpp
//  Kraken Engine
//
//  Copyright 2025 Kearwood Gilbert. All rights reserved.
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

bool smoke_load()
{
  enum
  {
    kSceneResourceHandle = 10,
    kSkyboxTextureResourceHandle,
    kCubeMeshResourceHandle
  };

  enum
  {
    kCameraNodeHandle = 10,
    kLightNodeHandle = 11,
    kCubeNodeHandle = 12
  };

  KrLoadResourceInfo load_resource_info = {};
  load_resource_info.sType = KR_STRUCTURE_TYPE_LOAD_RESOURCE;
  load_resource_info.resourceHandle = 1;
  load_resource_info.pResourcePath = "kraken_cube.krbundle";
  KrResult res = KrLoadResource(&load_resource_info);
  if (res != KR_SUCCESS) {
    //printf("Failed to load resource: %s\n", arg);
    KrShutdown();
    return false;
  }

  /*
  KrMapResourceInfo map_skybox_resource = { KR_STRUCTURE_TYPE_MAP_RESOURCE };
  map_skybox_resource.pResourceName = "skybox";
  map_skybox_resource.resourceHandle = kSkyboxTextureResourceHandle;
  KrResult res = KrMapResource(&map_skybox_resource);
  assert(res == KR_SUCCESS);
  */
  
  KrMapResourceInfo map_cube_mesh_resource = { KR_STRUCTURE_TYPE_MAP_RESOURCE };
  map_cube_mesh_resource.pResourceName = "__cube";
  map_cube_mesh_resource.resourceHandle = kCubeMeshResourceHandle;
  res = KrMapResource(&map_cube_mesh_resource);
  assert(res == KR_SUCCESS);

  // Create a scene
  KrCreateSceneInfo create_scene_info = { KR_STRUCTURE_TYPE_CREATE_SCENE };
  create_scene_info.resourceHandle = kSceneResourceHandle;
  create_scene_info.pSceneName = "my_scene";
  res = KrCreateScene(&create_scene_info);
  assert(res == KR_SUCCESS);

  hydra::Vector3 cameraPos = hydra::Vector3::Create(70, 0.5, 0.25);
  hydra::Matrix4 cameraMat = hydra::Matrix4::LookAt(cameraPos, hydra::Vector3::Create(0.0, 0.0, 0.0), hydra::Vector3::Up());
  hydra::Quaternion cameraRot = hydra::Quaternion::FromRotationMatrix(cameraMat);
  
  // Add a camera to the scene
  KrCreateNodeInfo create_camera_info = { KR_STRUCTURE_TYPE_CREATE_NODE };
  res = KrInitNodeInfo(&create_camera_info.node, KR_STRUCTURE_TYPE_NODE_CAMERA);
  assert(res == KR_SUCCESS);
  create_camera_info.relativeNodeHandle = KR_NULL_HANDLE;
  create_camera_info.location = KR_SCENE_NODE_APPEND_CHILD;
  create_camera_info.newNodeHandle = kCameraNodeHandle;
  create_camera_info.sceneHandle = kSceneResourceHandle;
  create_camera_info.node.pName = "default_camera";
  create_camera_info.node.camera.surfaceHandle = 0;
  create_camera_info.node.translate = cameraPos;
  create_camera_info.node.rotate = -cameraRot.eulerXYZ();
  // 
  // create_camera_info.node.translate = hydra::Vector3::Create(0.0, 5.0, 0.0);
  // create_camera_info.node.camera.skybox_texture = kSkyboxTextureResourceHandle;
  res = KrCreateNode(&create_camera_info);
  assert(res == KR_SUCCESS);
  
  
  /*
  // Add a light to the scene
  KrCreateNodeInfo create_light_info = { KR_STRUCTURE_TYPE_CREATE_NODE };
  res = KrInitNodeInfo(&create_light_info.node, KR_STRUCTURE_TYPE_NODE_DIRECTIONAL_LIGHT);
  assert(res == KR_SUCCESS);
  create_light_info.relativeNodeHandle = KR_NULL_HANDLE;
  create_light_info.location = KR_SCENE_NODE_APPEND_CHILD;
  create_light_info.newNodeHandle = kLightNodeHandle;
  create_light_info.sceneHandle = kSceneResourceHandle;
  create_light_info.node.pName = "my_light";
  create_light_info.node.rotate = hydra::Vector3::Create(0.25, 0.25, 0.37);
  res = KrCreateNode(&create_light_info);
  assert(res == KR_SUCCESS);
  */

  // Add a cube to the scene
  KrCreateNodeInfo create_cube_info = { KR_STRUCTURE_TYPE_CREATE_NODE };
  res = KrInitNodeInfo(&create_cube_info.node, KR_STRUCTURE_TYPE_NODE_MODEL);
  assert(res == KR_SUCCESS);
  create_cube_info.relativeNodeHandle = KR_NULL_HANDLE;
  create_cube_info.location = KR_SCENE_NODE_APPEND_CHILD;
  create_cube_info.newNodeHandle = kCubeNodeHandle;
  create_cube_info.sceneHandle = kSceneResourceHandle;
  create_cube_info.node.pName = "my_cube";
  create_cube_info.node.model.mesh = kCubeMeshResourceHandle;
  res = KrCreateNode(&create_cube_info);
  assert(res == KR_SUCCESS);
  
  for (int x = -5; x < 5; x++) {
    for (int y = -5; y < 5; y++) {
      for (int z = -5; z < 5; z++) {
        create_cube_info.node.translate.x = x * 10;
        create_cube_info.node.translate.y = y * 10;
        create_cube_info.node.translate.z = z * 10;
        res = KrCreateNode(&create_cube_info);
        assert(res == KR_SUCCESS);
      }
    }
  }
  
  return true;
}
