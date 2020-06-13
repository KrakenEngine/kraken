#include <windows.h>
#include "kraken.h"
#include "hello_cube.h"

using namespace kraken;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  MSG msg = { 0 };
  WNDCLASS wc = { 0 };
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
  wc.lpszClassName = L"krakensmoke";
  if (!RegisterClass(&wc))
    return 1;

  if (!CreateWindow(wc.lpszClassName,
    L"Kraken Smoke: Cube",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
    0, 0, 640, 480, 0, 0, hInstance, NULL))
    return 2;


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
  load_resource_info.pResourcePath = lpCmdLine; // "kraken_standard_assets.krbundle";
  res = KrLoadResource(&load_resource_info);
  if (res != KR_SUCCESS) {
	  //printf("Failed to load resource: %s\n", arg);
	  KrShutdown();
	  return 1;
  }

  smoke_load();

  while (GetMessage(&msg, NULL, 0, 0) > 0)
    DispatchMessage(&msg);

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
