#include "main.h"

#include <stdio.h>
#include "kraken.h"

using namespace kraken;

enum ResourceMapping {
  output_bundle = 0,
  loaded_resource = 1,
  shader_compile_log = 2,
};

int main( int argc, char *argv[] )
{
  bool failed = false;
  printf("Kraken Convert\n");
  printf("Initializing Kraken...\n");
  KrInitializeInfo init_info = {};
  init_info.sType = KR_STRUCTURE_TYPE_INITIALIZE;
  init_info.resourceMapSize = 1024;
  KrResult res = KrInitialize(&init_info);
  if (res != KR_SUCCESS) {
    printf("Failed to initialize Kraken!\n");
    return 1;
  }

  KrCreateBundleInfo create_bundle_info = {};
  create_bundle_info.sType = KR_STRUCTURE_TYPE_CREATE_BUNDLE;
  create_bundle_info.resourceHandle = ResourceMapping::output_bundle;
  create_bundle_info.pBundleName = "output";
  res = KrCreateBundle(&create_bundle_info);
  if (res != KR_SUCCESS) {
    printf("Failed to create bundle.\n");
    KrShutdown();
    return 1;
  }

  KrLoadResourceInfo load_resource_info = {};
  load_resource_info.sType = KR_STRUCTURE_TYPE_LOAD_RESOURCE;
  load_resource_info.resourceHandle = ResourceMapping::loaded_resource;

  KrMoveToBundleInfo move_to_bundle_info = {};
  move_to_bundle_info.sType = KR_STRUCTURE_TYPE_MOVE_TO_BUNDLE;
  move_to_bundle_info.bundleHandle = ResourceMapping::output_bundle;

  char* output_bundle = nullptr;
  bool compile_shaders = false;

  for (int i = 1; i < argc && !failed; i++) {
    char *arg = argv[i];
	  if (arg[0] == '-') {
	    continue;
	  }
	  if (i > 1 && argv[i - 1][0] == '-') {
      char command = argv[i - 1][1];
      switch (command) {
      case 'o':
        output_bundle = arg;
        break;
      case 'c':
        compile_shaders = true;
        break;
      default:
        printf("Unknown command: -%c\n", command);
        break;
      }
      continue;
	  }

    load_resource_info.pResourcePath = arg;
	  printf("loading %s... ", arg);
      res = KrLoadResource(&load_resource_info);
      if (res != KR_SUCCESS) {
        printf("[FAIL] (KrLoadResource)\n");
        failed = true;
		  continue;
      }
      move_to_bundle_info.resourceHandle = ResourceMapping::loaded_resource;
      res = KrMoveToBundle(&move_to_bundle_info);
      if (res != KR_SUCCESS) {
        printf("[FAIL] (KrMoveToBundle)\n");
        failed = true;
		  continue;
    }
	  printf("[GOOD]\n");
  }

  if (compile_shaders && !failed) {
    printf("Compiling Shaders...\n");
    KrCompileAllShadersInfo compile_all_shaders_info = {};
    compile_all_shaders_info.sType = KR_STRUCTURE_TYPE_COMPILE_ALL_SHADERS;
    compile_all_shaders_info.logHandle = ResourceMapping::shader_compile_log;
    res = KrCompileAllShaders(&compile_all_shaders_info);
    if (res != KR_SUCCESS) {
      printf("[FAIL] (Error %i)\n", res);
      failed = true;
    }
    else {
      printf("[GOOD]\n");
    }
  }

  if (output_bundle && !failed) {
    printf("Bundling %s... ", output_bundle);
    KrSaveResourceInfo save_resource_info = {};
    save_resource_info.sType = KR_STRUCTURE_TYPE_SAVE_RESOURCE;
    save_resource_info.resourceHandle = ResourceMapping::output_bundle;
    save_resource_info.pResourcePath = output_bundle;
    res = KrSaveResource(&save_resource_info);
    if (res != KR_SUCCESS) {
      printf("[FAIL] (Error %i)\n", res);
      failed = true;
    } else {
      printf("[GOOD]\n");
    }
  }

  KrShutdown();
  return failed ? 1 : 0;
}
