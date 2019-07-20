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
  init_info.pNext = NULL;
  KrResult res = KrInitialize(&init_info);
  if (res != KR_SUCCESS) {
    printf("Failed to initialize Kraken!\n");
    return 1;
  }

  KrLoadResourceInfo load_resource_info = {};

  for (int i = 0; i < argc; i++) {
    char *arg = argv[i];
    if (arg[0] != '-') {
      load_resource_info.pResourcePath = arg;
      res = KrLoadResource(&load_resource_info);
      if (res != KR_SUCCESS) {
        printf("Failed to load resource: %s\n", arg);
      }
    }
  }

  KrShutdown();
/*
  Context* context = Context::Get();

  for (int i = 0; i < argc; i++) {
    char *arg = argv[i];
    if (arg[0] != '-') {
      context->loadResource(arg);
    }
  }
*/
  return 0;
}
