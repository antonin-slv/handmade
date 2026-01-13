#include <Windows.h>
#include <stdint.h>

// TODO : Temporary (has to get mor grannular control over what is included)
static bool running;
static BITMAPINFO bitmapInfo;
static void *bitMapMemory;

static int BitmapWidth;
static int BitmapHeight;
static int BytesPerPixel = 4;


static void RenderGradient(int XOffset, int YOffset)
{
  int pitch = BitmapWidth * BytesPerPixel;
  uint8_t *row = (uint8_t *)bitMapMemory;
  for (int Y = 0; Y < BitmapHeight; ++Y)
  {
    uint32_t *pixel = (uint32_t *)row;
    for (int X = 0; X < BitmapWidth; ++X)
    {
      uint8_t blue = (X + XOffset) % 256;
      uint8_t green = (Y + YOffset) % 256;
      uint8_t red = 0;

      *pixel++ = ((red << 16) | (green << 8) | blue);
    }
    row += (BitmapWidth * BytesPerPixel);
  }
}

static void ResizeDIBSection(int Width, int Height)
{
  static bool firstTime;

  if (bitMapMemory)
  {
    VirtualFree(bitMapMemory, 0, MEM_RELEASE);
  }

  BitmapWidth = Width;
  BitmapHeight = Height;

  bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
  bitmapInfo.bmiHeader.biWidth = BitmapWidth;
  bitmapInfo.bmiHeader.biHeight = -BitmapHeight;
  bitmapInfo.bmiHeader.biPlanes = 1;
  bitmapInfo.bmiHeader.biBitCount = 32;
  bitmapInfo.bmiHeader.biCompression = BI_RGB;

  int BytesPerPixel = 4;
  int bitMapMemorySize = (BitmapWidth * BitmapHeight) * BytesPerPixel;
  bitMapMemory = VirtualAlloc(
      nullptr,
      bitMapMemorySize,
      MEM_RESERVE | MEM_COMMIT,
      PAGE_READWRITE);
}

static void MyUpdateWindow(
    HDC WindowContext, RECT *WindowRect,
    int X, int Y, int Width, int Height)
{
  int windowWidth = WindowRect->right - WindowRect->left;
  int windowHeight = WindowRect->bottom - WindowRect->top;

  StretchDIBits(
      WindowContext,
      0, 0, BitmapWidth, BitmapHeight,
      0, 0, windowWidth, windowHeight,
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

    MyUpdateWindow(DeviceContext, &lpPaint.rcPaint, X, Y, Width, Height);

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
      int xOffset = 0;
      int yOffset = 0;
      running = true;
      while (running)
      {
        MSG message;
        while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
        {
          if (message.message == WM_QUIT)
          {
            running = false;
          }
          TranslateMessage(&message);
          DispatchMessage(&message);
        }
        HDC DeviceContext = GetDC(window);
        RECT clientRect;
        GetClientRect(window, &clientRect);
        RenderGradient(xOffset++, yOffset);
        int windowWidth = clientRect.right - clientRect.left;
        int windowHeight = clientRect.bottom - clientRect.top;
        MyUpdateWindow(DeviceContext, &clientRect,0, 0, windowWidth, windowHeight);
        ReleaseDC(window, DeviceContext);
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