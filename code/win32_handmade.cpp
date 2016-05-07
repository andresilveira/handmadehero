#include <windows.h>

// getting rid of the three meanings of static depending on the scope
#define internal_function static
#define local_persist static
#define global_variable static

// global for the time being
global_variable bool running;
global_variable BITMAPINFO bitmapInfo;
global_variable void *bitmapMemory;
global_variable HBITMAP bitmapHandle;
global_variable HDC deviceContext;

internal_function void win32ResizeDIBSection(int width, int height) {

  // TODO: Maybe don't free first, free after, then free first if that fails.

  // free our DIBSection
  if (bitmapHandle) {
    DeleteObject(bitmapHandle);
  }
  if (!deviceContext) {
	  // TODO: should we recreate it?
	  deviceContext = CreateCompatibleDC(0);
  }

  bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
  bitmapInfo.bmiHeader.biWidth = width;
  bitmapInfo.bmiHeader.biHeight = height;
  bitmapInfo.bmiHeader.biPlanes = 1;
  bitmapInfo.bmiHeader.biBitCount = 32;
  bitmapInfo.bmiHeader.biCompression = BI_RGB;

  deviceContext = CreateCompatibleDC(0);

  bitmapHandle = CreateDIBSection(deviceContext, &bitmapInfo, DIB_RGB_COLORS, &bitmapMemory, 0, 0);
}

internal_function void win32UpdateWindow(HDC deviceContext, int x, int y, int width, int height) {
  // copy one rectangular buffer to another
  StretchDIBits(deviceContext,
    x, y, width, height, // destination
    x, y, width, height, // source
    bitmapMemory,
    &bitmapInfo,
    DIB_RGB_COLORS, SRCCOPY);
}

// Windows callback to handle different system messages like resizing,
// paiting, etc
LRESULT CALLBACK win32MainWindowCallback(
  HWND window,
  UINT message,
  WPARAM wParam,
  LPARAM lParam)
{
  LRESULT result = 0;

  switch (message) {
    case WM_SIZE: {
      RECT clientRect;
      GetClientRect(window, &clientRect);
      int width = clientRect.right - clientRect.left;
      int height = clientRect.bottom - clientRect.top;
      win32ResizeDIBSection(width, height);
      OutputDebugStringA("WM_SIZE\n");
    } break;
    case WM_DESTROY: {
      running = false;
      OutputDebugStringA("WM_DESTROY\n");
    } break;
    case WM_CLOSE: {
      running = false;
      OutputDebugStringA("WM_CLOSE\n");
    } break;
    case WM_ACTIVATEAPP: {
      OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;
    case WM_PAINT: { // paints two rectangles, black and white, on the window
      PAINTSTRUCT paint;
      HDC deviceContext = BeginPaint(window, &paint);

      int width = (paint.rcPaint.right - paint.rcPaint.left) / 2;
      int height = (paint.rcPaint.bottom - paint.rcPaint.top);

      int x = paint.rcPaint.left;
      int y = paint.rcPaint.top;

      win32UpdateWindow(deviceContext, x, y, width, height);

      PatBlt(deviceContext, x, y, width, height, BLACKNESS);

      EndPaint(window, &paint);
    } break;
    default: {
      OutputDebugStringA("default\n");
      result = DefWindowProc(window, message, wParam, lParam);
    } break;
  }

  return(result);
}

// Windows entry point
int CALLBACK WinMain(
  HINSTANCE instance,
  HINSTANCE prevInstance,
  LPSTR commandLine,
  int showCode)
{
  WNDCLASS windowClass = {};

  windowClass.lpfnWndProc = win32MainWindowCallback;
  windowClass.hInstance = instance;
  windowClass.lpszClassName = "HandmadeHeroWindowClass";

  if(RegisterClassA(&windowClass)){
    HWND windowHandle = CreateWindowExA(
      0,
      windowClass.lpszClassName,
      "Handmade Hero",
      WS_OVERLAPPEDWINDOW|WS_VISIBLE,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      0,
      0,
      instance,
      0
    );

    if (windowHandle) {
      running = true;
      MSG message;

      // ilustrative state loop
      while (running) {
        if(GetMessageA(&message, 0, 0, 0) < 0){ break; }

        TranslateMessage(&message);
        DispatchMessageA(&message);
      }
    } else {
      // TODO: log the failure
    }
  } else {
    // TODO: log the failure
  }

  return(0);
}
