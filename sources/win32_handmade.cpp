#include <Windows.h>

// TODO : Temporary (has to get mor grannular control over what is included)
static bool running;
static BITMAPINFO bitmapInfo;
static void *bitMapMemory;
static HBITMAP BitMapHandle;
static HDC BitMapDeviceContext;

static void ResizeDIBSection(int Width, int Height)
{
  if (BitMapHandle)
  {
    DeleteObject(BitMapHandle);
  }
  if (!BitMapDeviceContext)
  {
    BitMapDeviceContext = CreateCompatibleDC(0);
  }

  bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
  bitmapInfo.bmiHeader.biWidth = Width;
  bitmapInfo.bmiHeader.biHeight = -Height;
  bitmapInfo.bmiHeader.biPlanes = 1;
  bitmapInfo.bmiHeader.biBitCount = 32;
  bitmapInfo.bmiHeader.biCompression = BI_RGB;
  bitmapInfo.bmiHeader.biXPelsPerMeter = 96;
  bitmapInfo.bmiHeader.biYPelsPerMeter = 96;

  BitMapHandle = CreateDIBSection(
      BitMapDeviceContext,
      &bitmapInfo,
      DIB_RGB_COLORS,
      &bitMapMemory,
      nullptr, 0);
}

static void MyUpdateWindow(
    HDC WindowContext, int X, int Y,
    int Width, int Height)
{
  StretchDIBits(
      WindowContext,
      X, Y, Width, Height,
      0, 0, Width, Height,
      bitMapMemory,
      &bitmapInfo,
      DIB_RGB_COLORS, SRCCOPY);
}

LRESULT mainWindowCallback(
    HWND Window,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam)
{
  LRESULT rslt = 0;

  switch (Message)
  {
  case WM_DESTROY:
  {
    running = false;
    OutputDebugStringA("WM_DESTROY\n");
  }
  break;

  case WM_CLOSE:
  {
    running = false;
    OutputDebugStringA("WM_CLOSE\n");
  }
  break;

  case WM_SIZE:
  {
    RECT clientRect;

    GetClientRect(Window, &clientRect);
    int Height = clientRect.bottom - clientRect.top;
    int Width = clientRect.right - clientRect.left;

    ResizeDIBSection(
        LOWORD(lParam),
        HIWORD(lParam));
    OutputDebugStringA("WM_SIZE\n");
  }
  break;

  case WM_ACTIVATEAPP:
  {
    OutputDebugStringA("WM_ACTIVATEAPP\n");
  }
  break;

  case WM_PAINT:
  {
    PAINTSTRUCT lpPaint;
    HDC DeviceContext = BeginPaint(Window, &lpPaint);

    int Height = lpPaint.rcPaint.bottom - lpPaint.rcPaint.top;
    int Width = lpPaint.rcPaint.right - lpPaint.rcPaint.left;
    int X = lpPaint.rcPaint.left;
    int Y = lpPaint.rcPaint.top;

    MyUpdateWindow(DeviceContext, X, Y, Width, Height);

    EndPaint(Window, &lpPaint);
  }
  break;

  default:
  {
    OutputDebugStringA("default\n");
    rslt = DefWindowProcA(Window, Message, wParam, lParam);
  }
  }
  return rslt;
}

int WINAPI WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int nShowCmd)
{
  WNDCLASS windowClass = {};

  windowClass.style = CS_CLASSDC | CS_HREDRAW | CS_VREDRAW;
  windowClass.lpfnWndProc = mainWindowCallback;
  windowClass.hInstance = Instance;
  // HICON     windowClass.hIcon;
  LPCWSTR lpszMenuName;
  windowClass.lpszClassName = "HandmadeWindowClass";

  if (RegisterClass(&windowClass))
  {
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
        nullptr);

    if (window)
    {
      MSG message;
      BOOL getMessageResult;
      running = true;
      while ((getMessageResult = GetMessage(&message, nullptr, 0, 0)) > 0 && running)
      {
        TranslateMessage(&message);
        DispatchMessage(&message);
      }
    }
    else
    {
      OutputDebugStringA("Failed to create window.\n");
    }
  }
  else
  {
    OutputDebugStringA("Failed to register window class.\n");
  }

  return 0;
}