//
//  main_win.cpp
//  Kraken Engine
//
//  Copyright 2021 Kearwood Gilbert. All rights reserved.
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

#include <windows.h>
#include <shellscalingapi.h>
#include "kraken.h"
#include "hello_cube.h"

using namespace kraken;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

  MSG msg = { 0 };
  WNDCLASS wc = { 0 };
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
  wc.lpszClassName = L"krakensmoke";
  if (!RegisterClass(&wc))
    return 1;

  HWND hWnd = CreateWindow(wc.lpszClassName,
    L"Kraken Smoke: Cube",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
    0, 0, 640, 480, 0, 0, hInstance, NULL);

  if (!hWnd) {
    return 2;
  }

  KrInitializeInfo init_info = {};
  init_info.sType = KR_STRUCTURE_TYPE_INITIALIZE;
  init_info.resourceMapSize = 1024;
  KrResult res = KrInitialize(&init_info);
  if (res != KR_SUCCESS) {
	  // printf("Failed to initialize Kraken!\n");
	  return 1;
  }

  KrLoadResourceInfo load_resource_info = {};
  load_resource_info.sType = KR_STRUCTURE_TYPE_LOAD_RESOURCE;
  load_resource_info.resourceHandle = 1;
  load_resource_info.pResourcePath = "kraken_cube.krbundle";
  res = KrLoadResource(&load_resource_info);
  if (res != KR_SUCCESS) {
	  //printf("Failed to load resource: %s\n", arg);
	  KrShutdown();
	  return 1;
  }

  KrCreateWindowSurfaceInfo create_surface_info = {};
  create_surface_info.sType = KR_STRUCTURE_TYPE_CREATE_WINDOW_SURFACE;
  create_surface_info.surfaceHandle = 1;
  create_surface_info.hWnd = static_cast<void*>(hWnd);
  res = KrCreateWindowSurface(&create_surface_info);
  if (res != KR_SUCCESS) {
    //printf("Failed to create window surface.\n");
    KrShutdown();
    return 1;
  }

  smoke_load();

  while (GetMessage(&msg, NULL, 0, 0) > 0) {
    DispatchMessage(&msg);
  }
    

  // KrShutdown will delete the window surfaces for us; however, we
  // include this here for code coverage in tests.
  KrDeleteWindowSurfaceInfo delete_surface_info = {};
  delete_surface_info.sType = KR_STRUCTURE_TYPE_DELETE_WINDOW_SURFACE;
  delete_surface_info.surfaceHandle = 1;
  res = KrDeleteWindowSurface(&delete_surface_info);
  if (res != KR_SUCCESS) {
    //printf("Failed to delete window surface.\n");
    KrShutdown();
    return 1;
  }

  KrShutdown();

  return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

  switch (message)
  {
  case WM_CLOSE:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;

}
