#include <windows.h>

// Windows callback to handle different system messages like resizing,
// paiting, etc
LRESULT CALLBACK mainWindowCallback(
  HWND window,
  UINT message,
  WPARAM wParam,
  LPARAM lParam)
{
  LRESULT result = 0;

  switch (message) {
    case WM_SIZE: {
      OutputDebugStringA("WM_SIZE\n");
    } break;
    case WM_DESTROY: {
      OutputDebugStringA("WM_DESTROY\n");
    } break;
    case WM_CLOSE: {
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

      int x1 = paint.rcPaint.left;
      int y1 = paint.rcPaint.top;

      int x2 = paint.rcPaint.left + width;
      int y2 = paint.rcPaint.top;

      PatBlt(deviceContext, x1, y1, width, height, BLACKNESS);
      PatBlt(deviceContext, x2, y2, width, height, WHITENESS);

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

  windowClass.lpfnWndProc = mainWindowCallback;
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
      MSG message;

      // ilustrative state loop
      for (;;) {
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
