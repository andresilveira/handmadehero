#include <windows.h>
#include <stdint.h>

// getting rid of the three meanings of static depending on the scope
#define internal_function static
#define local_persist static
#define global_variable static

// global for the time being
global_variable bool running;
global_variable BITMAPINFO bitmapInfo;
global_variable void *bitmapMemory;
global_variable int bitmapWidth;
global_variable int bitmapHeight;
global_variable const int BIT_DEPTH = 32;
global_variable int bytesPerPixel = BIT_DEPTH / 8;

internal_function void renderWeirdGradient(int xOffset, int yOffset){
  int pitch = bitmapWidth * bytesPerPixel; // the indicator of the size of a row
  uint8_t *row = (uint8_t *)bitmapMemory;
  for (size_t y = 0; y < bitmapHeight; y++) {
    uint32_t *pixel = (uint32_t *)row;
    for (size_t x = 0; x < bitmapWidth; x++) {
      // pixel in memory (hex), in a little endian machine, running windows...
      // 00(B) 00(G) 00(R) 00(padding)
      uint8_t red = 0;
      uint8_t green = x + xOffset;
      uint8_t blue = y + yOffset;
      *pixel++ = blue | (green << 8) | (red << 16);
    }
    row += pitch;
  }
}

internal_function void win32ResizeDIBSection(int width, int height) {
  // release the bitmapMemory if we have gotten it
  if (bitmapMemory) {
    VirtualFree(bitmapMemory, 0, MEM_RELEASE);
  }

  bitmapWidth = width;
  bitmapHeight = height;

  // if biHeight is negative the starting point for the DIB is top-left otherwise, bottom-left
  bitmapInfo.bmiHeader.biHeight = -bitmapHeight;
  bitmapInfo.bmiHeader.biWidth = bitmapWidth;
  bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
  bitmapInfo.bmiHeader.biPlanes = 1;
  bitmapInfo.bmiHeader.biBitCount = BIT_DEPTH;
  bitmapInfo.bmiHeader.biCompression = BI_RGB;


  int bitmapMemorySize = width * height * bytesPerPixel;
  bitmapMemory =  VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal_function void win32UpdateWindow(HDC deviceContext, RECT *windowRect, int x, int y, int width, int height) {
  int windowWidth = windowRect->right - windowRect->left;
  int windowHeight = windowRect->bottom - windowRect->top;

  // copy one rectangular buffer to another
  StretchDIBits(deviceContext,
    0, 0, bitmapWidth, bitmapHeight, // destination
    0, 0, windowWidth, windowHeight, // source
    bitmapMemory,
    &bitmapInfo,
    DIB_RGB_COLORS, SRCCOPY);
}

// Windows callback to handle different system messages like resizing,
// paiting, etc
LRESULT CALLBACK win32MainWindowCallback(
  HWND windowHandle,
  UINT message,
  WPARAM wParam,
  LPARAM lParam)
{
  LRESULT result = 0;

  switch (message) {
    case WM_SIZE: {
      RECT clientRect;
      GetClientRect(windowHandle, &clientRect);
      int width = clientRect.right - clientRect.left;
      int height = clientRect.bottom - clientRect.top;
      win32ResizeDIBSection(width, height);
    } break;
    case WM_DESTROY: {
      running = false;
      OutputDebugStringA("WM_DESTROY\n");
    } break;
    case WM_CLOSE: {
      running = false;
    } break;
    case WM_ACTIVATEAPP: {
      OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;
    case WM_PAINT: { // paints two rectangles, black and white, on the window
      PAINTSTRUCT paint;
      HDC deviceContext = BeginPaint(windowHandle, &paint);
      RECT clientRect; GetClientRect(windowHandle, &clientRect);
      int width = clientRect.right - clientRect.left;
      int height = clientRect.bottom - clientRect.top;
      win32UpdateWindow(deviceContext, &clientRect, clientRect.right, clientRect.top, width, height);
      EndPaint(windowHandle, &paint);
    } break;
    default: {
      OutputDebugStringA("default\n");
      result = DefWindowProc(windowHandle, message, wParam, lParam);
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
      int xOffset = 0;
      int yOffset = 0;

      // ilustrative state loop
      while (running) {
        MSG message;
        while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
          if (message.message == WM_QUIT) { running = false; }

          TranslateMessage(&message);
          DispatchMessageA(&message);
        }

        HDC deviceContext = GetDC(windowHandle);
        RECT clientRect; GetClientRect(windowHandle, &clientRect);
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;
        win32UpdateWindow(deviceContext, &clientRect, clientRect.right, clientRect.top, width, height);
        renderWeirdGradient(xOffset, yOffset);
        ReleaseDC(windowHandle, deviceContext);

        ++xOffset;
        ++yOffset;
      }
    } else {
      // TODO: log the failure
    }
  } else {
    // TODO: log the failure
  }

  return(0);
}
