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
#include "win32_renderVisual.cpp"

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
static float zoom_level = 1.0f;

static Win32WindowDimension Win32GetWindowDimension(HWND Window)
{
  Win32WindowDimension result;

  RECT clientRect;
  GetClientRect(Window, &clientRect);
  result.Width = clientRect.right - clientRect.left;
  result.Height = clientRect.bottom - clientRect.top;

  return result;
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
  case WM_MOUSEWHEEL:
  {
    MouseState.wheel_delta = GET_WHEEL_DELTA_WPARAM(wParam);
  }

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
// after day 9 :
// 2.432700 ms per frames for full debug build
// 1.721500 with optimisation on
int WINAPI WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int nShowCmd)
{
  WNDCLASS windowClass = {};
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
      CW_USEDEFAULT, CW_USEDEFAULT,
      CW_USEDEFAULT, CW_USEDEFAULT,
      nullptr,
      nullptr,
      Instance,
      nullptr);

  if (!window)
  {
    OutputDebugStringA("Failed to create window.\n");
    return -1; // ----------------------------------------------------- RETURN
  }
  // image buffer initialization
  Win32WindowDimension windowDim = Win32GetWindowDimension(window);
  // made here to match the initial window size to avoid stretching
  //  TODO : should be initialized earlier with better stretching algorithm)
  ResizeDIBSection(&globalBackBuffer, windowDim.Width, windowDim.Height);

  int array_width = 2000;
  int array_height = 2000;
  int *test_array = (int *)malloc(array_width * array_height * sizeof(int));
  for (int y = 0; y < array_height; y++)
  {
    for (int x = 0; x < array_width; x++)
    {
      uint8_t red = x * 5 % 256;
      uint8_t green = y * 5 % 256;
      uint8_t blue = 0;
      test_array[y * array_width + x] = (red << 16) | (green << 8) | blue;
    }
  }

  // audio initialization
  bool hasAudio = false;
  IAudioRenderClient *pRenderAudioClient;
  WAVEFORMATEX *pwfx;

  DWORD audioFlags = 0;
  int wavetime = 0;

  HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

  Win32SoundOutput SoundStat = {};

  if (SUCCEEDED(hr))
  {
    hr = win32_GetAudioRenderClient(&pRenderAudioClient, &pwfx);

    if (SUCCEEDED(hr))
    {

      SoundStat.SampleRate = pwfx->nSamplesPerSec;
      SoundStat.Frequency = 440.0f;
      SoundStat.Volume = 0.5f;
      SoundStat.SampleIndex = 0;
      SoundStat.WaveShape = WAVE_SHAPE_SINE;

      hasAudio = true;
    }
    Win32FillAudioBuffer(*pAudioClient, pRenderAudioClient, audioFlags, SoundStat);

    hr = pAudioClient->Start();
  }
  // performance - - - - -
  LARGE_INTEGER Frequency;
  QueryPerformanceFrequency(&Frequency);
  int64_t PerfCountFrequency = Frequency.QuadPart;
  LARGE_INTEGER LastCounter;
  LARGE_INTEGER StartCounter;
  QueryPerformanceCounter(&LastCounter);
  long long avgFrameTime = 0;

  // actual loop
  int xOffset = 0;
  int yOffset = 0;
  running = true;
  uint64_t framCount = 0;
  while (running)
  {
    float deltaT;
    QueryPerformanceCounter(&StartCounter);
    deltaT = (float)(StartCounter.QuadPart - LastCounter.QuadPart) / (float)PerfCountFrequency;
    avgFrameTime = (avgFrameTime * framCount + (long long)(StartCounter.QuadPart - LastCounter.QuadPart)) / (long long)(framCount + 1);
    LastCounter = StartCounter;

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

    if (MouseState.is_left_down())
    {
      xOffset -= (MouseState.x - MouseState.last_x);
      yOffset -= (MouseState.y - MouseState.last_y);
    }
    if (MouseState.wheel_delta != 0)
    {
      float prev_zoom = zoom_level;
      zoom_level *= (1.0f + (float)MouseState.wheel_delta / 4000.0f);

      // pour centrer le zoom : combien de pixels sont ajoutés / retirés ?
      //  si zoom_level augmente, on crop dans l'image, donc on enlève des pixels
      float pixel_change_x = (zoom_level / prev_zoom) * (globalBackBuffer.Width / 2 + xOffset) - (globalBackBuffer.Width / 2);
      float pixel_change_y = (zoom_level / prev_zoom) * (globalBackBuffer.Height / 2 + yOffset) - (globalBackBuffer.Height / 2);

      xOffset = (int)pixel_change_x;
      yOffset = (int)pixel_change_y;
    }
    // RenderGradient(&globalBackBuffer, xOffset, yOffset);
    // renderCheckerboard(&globalBackBuffer, 4, xOffset, yOffset, 0, 0);
    renderArrayPattern(&globalBackBuffer, test_array, array_width, array_height, 1.0f, xOffset, yOffset, zoom_level);

    float fps = 1.0f / deltaT;
    char fps_buffer[256];
    sprintf_s(fps_buffer, "FPS: %f\tMS: %f", fps, deltaT * 1000.0f);
    renderString(&globalBackBuffer, fps_buffer, 10, 10);
    printf("%s\n", fps_buffer);

    Win32WindowDimension Dimension = Win32GetWindowDimension(window);

    Win32CopyBufferToWindow(DeviceContext, Dimension.Width, Dimension.Height, &globalBackBuffer, 0, 0, Dimension.Width, Dimension.Height);
    ReleaseDC(window, DeviceContext);
    // continue audio streaming
    if (hasAudio)
    {
      // TODO : rajouter une gestion de la latence audio ici...
      // actuellement 1 seconde, mais on veut quelque chose de réactif
      Win32FillAudioBuffer(*pAudioClient, pRenderAudioClient, audioFlags, SoundStat);
    }

    // clears mouse movement delta
    MouseState.last_x = MouseState.x;
    MouseState.last_y = MouseState.y;
    MouseState.wheel_delta = 0;

    // advance frame index
    framCount++;
  }

  OutputDebugStringA("Average frame time (ms): ");
  char avgFrameTimeBuffer[256];
  sprintf_s(avgFrameTimeBuffer, "%f\n", (float)((avgFrameTime * 1000.0f) / (double)PerfCountFrequency));
  OutputDebugStringA(avgFrameTimeBuffer);
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
