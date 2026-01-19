#include <Windows.h>
#include <Windowsx.h>

#include <string>
#include <stdint.h>

#include <Xinput.h>
#include <audioclient.h>
#include <Audiopolicy.h>

#include "win32_audio_engine.cpp"
#include "win32_controller.cpp"
#include "win32_keyboard.h"

struct Win32OffscreenBuffer
{
  BITMAPINFO Info;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};

struct Win32WindowDimension
{
  int Width;
  int Height;
};

static Win32OffscreenBuffer globalBackBuffer;
// TODO : Temporary (has to get mor grannular control over what is included)
static bool running;

// TODO : Temporary static
static keyboard_state KeyboardState;
static mouse_state MouseState;

static Win32WindowDimension Win32GetWindowDimension(HWND Window)
{
  Win32WindowDimension result;

  RECT clientRect;
  GetClientRect(Window, &clientRect);
  result.Width = clientRect.right - clientRect.left;
  result.Height = clientRect.bottom - clientRect.top;

  return result;
}
static void RenderGradient(Win32OffscreenBuffer *Buffer, int XOffset, int YOffset)
{
  uint8_t *row = (uint8_t *)Buffer->Memory;
  for (int Y = 0; Y < Buffer->Height; ++Y)
  {
    uint32_t *pixel = (uint32_t *)row;
    for (int X = 0; X < Buffer->Width; ++X)
    {
      uint8_t blue = (X + XOffset) % 256;
      uint8_t green = (Y + YOffset) % 256;
      uint8_t red = 0;

      *pixel++ = ((red << 16) | (green << 8) | blue);
    }
    row += Buffer->Pitch;
  }
}

static void ResizeDIBSection(Win32OffscreenBuffer *Buffer, int Width, int Height)
{
  static bool firstTime;

  if (Buffer->Memory)
  {
    VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
  }

  Buffer->Width = Width;
  Buffer->Height = Height;
  Buffer->BytesPerPixel = 4;

  Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
  Buffer->Info.bmiHeader.biWidth = Buffer->Width;
  Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
  Buffer->Info.bmiHeader.biPlanes = 1;
  Buffer->Info.bmiHeader.biBitCount = 32;
  Buffer->Info.bmiHeader.biCompression = BI_RGB;
  Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel;

  int bitMapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;
  Buffer->Memory = VirtualAlloc(
      nullptr,
      bitMapMemorySize,
      MEM_RESERVE | MEM_COMMIT,
      PAGE_READWRITE);
}

static void Win32CopyBufferToWindow(
    HDC WindowContext, int WindowWidth, int WindowHeight,
    Win32OffscreenBuffer *Buffer,
    int X, int Y, int Width, int Height)
{

  StretchDIBits(
      WindowContext,
      0, 0, WindowWidth, WindowHeight,
      0, 0, Buffer->Width, Buffer->Height,
      Buffer->Memory,
      &Buffer->Info,
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
    Win32WindowDimension window = Win32GetWindowDimension(Window);
    Win32CopyBufferToWindow(DeviceContext, window.Width, window.Height, &globalBackBuffer, X, Y, Width, Height);

    EndPaint(Window, &lpPaint);
  }
  break;

  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_KEYDOWN:
  case WM_KEYUP:
  {
    uint32_t VKCode = wParam;

    WORD keyFlags = HIWORD(lParam);
    bool isKeyReleased = (keyFlags & KF_UP) != KF_UP;
    KeyboardState.set_key_state((int)VKCode, isKeyReleased);

    bool wasDown = (keyFlags & KF_REPEAT) != KF_REPEAT;
    bool altKeyWasDown = (lParam & (1 << 29)) != 0;

    if (altKeyWasDown && VKCode == VK_F4)
    {
      running = false;
    }
  }
  break;

  case WM_MOUSEMOVE:
  {
    MouseState.last_x = MouseState.x;
    MouseState.last_y = MouseState.y;

    MouseState.x = GET_X_LPARAM(lParam);
    MouseState.y = GET_Y_LPARAM(lParam);
  }
  break;

  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
  case WM_XBUTTONDOWN:
  case WM_XBUTTONUP:
  {
    MouseState.set_mouse((uint32_t)wParam);
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

  ResizeDIBSection(&globalBackBuffer, 1280, 720);

  windowClass.style = CS_HREDRAW | CS_VREDRAW;
  windowClass.lpfnWndProc = mainWindowCallback;
  windowClass.hInstance = Instance;
  // HICON     windowClass.hIcon;
  LPCWSTR lpszMenuName;
  windowClass.lpszClassName = "HandmadeWindowClass";

  if (!RegisterClass(&windowClass))
  {
    OutputDebugStringA("Failed to register window class.\n");
    return -1; // ----------------------------------------------------- RETURN
  }

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

  if (!window)
  {
    OutputDebugStringA("Failed to create window.\n");
    return -1; // ----------------------------------------------------- RETURN
  }

  // audio initialization
  bool hasAudio = false;
  IAudioRenderClient *pRenderAudioClient;
  WAVEFORMATEX *pwfx;

  DWORD audioFlags = 0;
  int wavetime = 0;

  HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (SUCCEEDED(hr))
  {
    hr = win32_GetRenderClient(&pRenderAudioClient, &pwfx);
    if (SUCCEEDED(hr))
    {
      hasAudio = true;
    }

    //

    fillAudioBuffer(*pAudioClient, pRenderAudioClient,
                    pwfx, audioFlags, wavetime);

    hr = pAudioClient->Start();
  }

  // actual loop
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

    for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex)
    {
      XINPUT_STATE controllerState;
      ZeroMemory(&controllerState, sizeof(XINPUT_STATE));

      if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
      {
        // Controller is connected
        Win32ControllerInput pad(controllerState.Gamepad);
      }
      else
      {
        // Controller is not connected
        // TODO :  this case will have to be handled in the future
      }
    }

    HDC DeviceContext = GetDC(window);
    RECT clientRect;
    GetClientRect(window, &clientRect);
    if (KeyboardState.is_key_down(VK_LEFT))
      xOffset -= 5;
    else if (KeyboardState.is_key_down(VK_RIGHT))
      xOffset += 5;
    else if (
        KeyboardState.is_key_down(KeyboardState.key_left))
    {
      xOffset -= 1;
    }
    else if (
        KeyboardState.is_key_down(KeyboardState.key_right))
    {
      xOffset += 1;
    }
    if (
        KeyboardState.is_key_down(KeyboardState.key_up))
    {
      yOffset -= 1;
    }
    else if (
        KeyboardState.is_key_down(KeyboardState.key_down))
    {
      yOffset += 1;
    }
    else if (KeyboardState.is_key_down(VK_UP))
      yOffset -= 5;
    else if (KeyboardState.is_key_down(VK_DOWN))
      yOffset += 5;

    if (MouseState.is_left_down())
    {
      xOffset -= (MouseState.x - MouseState.last_x);
      yOffset -= (MouseState.y - MouseState.last_y);
    }

    RenderGradient(&globalBackBuffer, xOffset, yOffset);

    Win32WindowDimension Dimension = Win32GetWindowDimension(window);

    Win32CopyBufferToWindow(DeviceContext, Dimension.Width, Dimension.Height, &globalBackBuffer, 0, 0, Dimension.Width, Dimension.Height);
    ReleaseDC(window, DeviceContext);
    // continue audio streaming
    if (hasAudio)
    {

      fillAudioBuffer(*pAudioClient, pRenderAudioClient,
                      pwfx, audioFlags, wavetime);
    }

    // clears mouse movement delta
    MouseState.last_x = MouseState.x;
    MouseState.last_y = MouseState.y;
  }

  // free audio resources (probably not necessary since the program is ending)
  if (hasAudio)
  {
    pAudioClient->Stop();
    pRenderAudioClient->Release();
    win32_releaseALLAudioClient();
  }
  CoUninitialize();

  return 0;
}
