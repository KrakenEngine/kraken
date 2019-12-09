#include "main.h"

#include <stdio.h>
#include "kraken.h"

using namespace kraken;

int main( int argc, char *argv[] )
{
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
  create_bundle_info.resourceHandle = 0;
  create_bundle_info.pBundleName = "output";
  res = KrCreateBundle(&create_bundle_info);
  if (res != KR_SUCCESS) {
    printf("Failed to create bundle.\n");
    KrShutdown();
    return 1;
  }

  KrLoadResourceInfo load_resource_info = {};
  load_resource_info.sType = KR_STRUCTURE_TYPE_LOAD_RESOURCE;
  load_resource_info.resourceHandle = 1;

  KrMoveToBundleInfo move_to_bundle_info = {};
  move_to_bundle_info.sType = KR_STRUCTURE_TYPE_MOVE_TO_BUNDLE;
  move_to_bundle_info.bundleHandle = 0;

  char* output_bundle = nullptr;

  for (int i = 1; i < argc; i++) {
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
		  continue;
      }
      move_to_bundle_info.resourceHandle = 1;
      res = KrMoveToBundle(&move_to_bundle_info);
      if (res != KR_SUCCESS) {
        printf("[FAIL] (KrMoveToBundle)\n");
		  continue;
    }
	  printf("[GOOD]\n");
  }

  if (output_bundle) {
    printf("Bundling %s... ", output_bundle);
    KrSaveResourceInfo save_resource_info = {};
    save_resource_info.sType = KR_STRUCTURE_TYPE_SAVE_RESOURCE;
    save_resource_info.resourceHandle = 0;
    save_resource_info.pResourcePath = output_bundle;
    res = KrSaveResource(&save_resource_info);
    if (res != KR_SUCCESS) {
      printf("[FAIL] (Error %i)\n", res);
    } else {
      printf("[GOOD]\n");
    }
  }

  KrShutdown();
  return 0;
}
