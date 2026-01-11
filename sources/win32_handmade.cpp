
#include <Windows.h>

WNDPROC Wndproc;

LRESULT mainWindowCallback(
  HWND Window,
  UINT Message,
  WPARAM wParam,
  LPARAM lParam
) {
LRESULT rslt = 0;

  switch (Message) {
    case WM_DESTROY: {
      OutputDebugStringA("WM_DESTROY\n");
    } break;
    
    case WM_CLOSE: {
      OutputDebugStringA("WM_CLOSE\n");
    } break;
    
    case WM_SIZE: {
      OutputDebugStringA("WM_SIZE\n");
    } break;
    
    case WM_ACTIVATEAPP: {
      OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;

    case WM_PAINT: {
      PAINTSTRUCT lpPaint;
      HDC DeviceContext = BeginPaint(Window, &lpPaint);

      LONG Height = lpPaint.rcPaint.bottom - lpPaint.rcPaint.top;
      LONG Width  = lpPaint.rcPaint.right  - lpPaint.rcPaint.left;

      PatBlt(
        DeviceContext,
        lpPaint.rcPaint.left,
        lpPaint.rcPaint.top,
        Width,
        Height,
        BLACKNESS
      );

      EndPaint(Window, &lpPaint);
    } break;

    default: {
      OutputDebugStringA("default\n");
      rslt = DefWindowProcA(Window, Message, wParam, lParam);
    }
  }
  return rslt;
}

int WINAPI WinMain(
  HINSTANCE Instance,
  HINSTANCE PrevInstance,
  LPSTR     CommandLine,
  int       nShowCmd
) {
  WNDCLASS windowClass = {};

  windowClass.style = CS_CLASSDC | CS_HREDRAW | CS_VREDRAW;
  windowClass.lpfnWndProc = mainWindowCallback;
  windowClass.hInstance = Instance;
  //HICON     windowClass.hIcon;
  LPCWSTR   lpszMenuName;
  windowClass.lpszClassName = "HandmadeWindowClass";

  if (RegisterClass(&windowClass)) {
    HWND window = CreateWindowEx(
      0,
      windowClass.lpszClassName,
      "Handmade",
      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      nullptr,
      nullptr,
      Instance,
      nullptr
    );

    if (window) {
      MSG message;
      BOOL getMessageResult;

      while ((getMessageResult = GetMessage(&message, nullptr, 0, 0)) > 0) {
        TranslateMessage(&message);
        DispatchMessage(&message);
      }
    } else {
      OutputDebugStringA("Failed to create window.\n");
    }
  } else {
    OutputDebugStringA("Failed to register window class.\n");
  }

  return 0;
}