#include "main.h"

#include <stdio.h>
#include "kraken.h"

using namespace kraken;

int main( int argc, char *argv[] )
{
  Context* context = Context::Get();

  for (int i = 0; i < argc; i++) {
    char *arg = argv[i];
    if (arg[0] != '-') {
      context->loadResource(arg);
    }
  }

  printf("Kraken Convert\n");
  return 0;
}
