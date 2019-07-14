#include <windows.h>
#include "kraken.h"

using namespace kraken;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  Context* context = Context::Get();

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

  while (GetMessage(&msg, NULL, 0, 0) > 0)
    DispatchMessage(&msg);

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
