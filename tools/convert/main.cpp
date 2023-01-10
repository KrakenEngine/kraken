//
//  main.cpp
//  Kraken Engine
//
//  Copyright 2023 Kearwood Gilbert. All rights reserved.
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
#include "main.h"

#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "kraken.h"

using namespace kraken;

enum ResourceMapping
{
  output_bundle = 0,
  loaded_resource = 1,
  shader_compile_log = 2,
};

int main(int argc, char* argv[])
{
  bool failed = false;
  printf("Kraken Convert\n");
  printf("Initializing Kraken...\n");
  KrInitializeInfo init_info = {};
  init_info.sType = KR_STRUCTURE_TYPE_INITIALIZE;
  init_info.resourceMapSize = 1024;
  init_info.nodeMapSize = 1024;
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
  char* input_list_file = nullptr;

  std::vector<std::string> input_files;

  char command = '\0';
  for (int i = 1; i < argc && !failed; i++) {
    char* arg = argv[i];
    if (arg[0] == '-') {
      if (command != '\0') {
        // The last command is expecting a parameter, not another command.
        printf("Invalid syntax. '%s' not expected after '-%c'\n", arg, command);
        failed = true;
        continue;
      }
      if (arg[1] == '\0') {
        // We received a lonely '-'
        printf("Invalid syntax. '-' must be followed by a command.\n");
        failed = true;
        continue;
      }
      if (arg[2] != '\0') {
        // All commands are currently one character long
        printf("Unknown command: '%s'\n", arg);
        failed = true;
        continue;
      }
      command = arg[1];
      switch (command) {
      case 'c':
        compile_shaders = true;
        command = '\0';
        break;
      case 'i':
      case 'o':
        // Next arg will be the output path
        break;
      default:
        printf("Unknown command: '%s'\n", arg);
        failed = true;
        continue;
      }
      continue;
    }

    // Process commands that receive arguments
    switch (command) {
    case 'i':
      input_list_file = arg;
      command = '\0';
      continue;
    case 'o':
      output_bundle = arg;
      command = '\0';
      continue;
    }

    input_files.push_back(arg);
  }

  if (input_list_file != nullptr) {
    printf("Reading %s... ", input_list_file);
    std::ifstream in(input_list_file);
    if (!in) {
      printf("[FAIL]\n");
      failed = true;
    } else {
      for (std::string line; std::getline(in, line); ) {
        const std::string ws = "\t ";
        line.erase(0, line.find_first_not_of(ws));
        line.erase(line.find_last_not_of(ws) + 1);
        if (!line.empty()) {
          input_files.push_back(line);
        }
      }
    }
    printf("[GOOD]\n");
  }

  for (const std::string& file_name : input_files) {
    load_resource_info.pResourcePath = file_name.c_str();
    printf("loading %s... ", load_resource_info.pResourcePath);
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
    printf("Compiling Shaders... ");
    KrCompileAllShadersInfo compile_all_shaders_info = {};
    compile_all_shaders_info.sType = KR_STRUCTURE_TYPE_COMPILE_ALL_SHADERS;
    compile_all_shaders_info.bundleHandle = ResourceMapping::output_bundle;
    compile_all_shaders_info.logHandle = ResourceMapping::shader_compile_log;
    res = KrCompileAllShaders(&compile_all_shaders_info);
    if (res != KR_SUCCESS) {
      printf("[FAIL] (Error %i)\n", res);
      failed = true;
    } else {
      printf("[GOOD]\n");
    }
    KrGetResourceDataInfo get_resource_data_info = {};
    get_resource_data_info.sType = KR_STRUCTURE_TYPE_GET_RESOURCE_DATA;
    get_resource_data_info.resourceHandle = ResourceMapping::shader_compile_log;
    res = KrGetResourceData(&get_resource_data_info, [](const KrGetResourceDataResult& result) {
      // TODO - This will later be asynchronous...  Will need to block rest of execution until returned
      if (result.result != KR_SUCCESS) {
        printf("Failed to get shader compile log.  (Error %i)\n", result.result);
      } else {
        // result.data will be a null terminated string
        if (result.data != nullptr && result.length > 0) {
          printf("Shader compile log:\n%s\n", static_cast<char*>(result.data));
        }
      }
    });
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
