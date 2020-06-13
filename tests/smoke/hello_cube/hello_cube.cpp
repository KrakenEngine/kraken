#include "hello_cube.h"
#include "kraken.h"
#include <assert.h>

void smoke_load()
{
  KrCreateSceneInfo create_scene_info = {};
  create_scene_info.sType = KR_STRUCTURE_TYPE_CREATE_SCENE;
  create_scene_info.resourceHandle = 10;
  create_scene_info.pSceneName = "cube";
  KrResult res = KrCreateScene(&create_scene_info);
  assert(res == KR_SUCCESS);
}
