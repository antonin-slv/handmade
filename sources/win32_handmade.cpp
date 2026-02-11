#include <Windows.h>
#include <Windowsx.h>

#include <string>
#include <stdint.h>

#include <Xinput.h>
#include <audioclient.h>
#include <Audiopolicy.h>

#include "win32_handmade.h"

#include "win32_audioLayer.cpp"
#include "win32_controller.cpp"

#include "hand_keyboard.h"
#include "handmade.cpp"

static HandmadeScreenBuffer globalBackBuffer;
static BITMAPINFO globalBackBufferInfo;
// TODO : Temporary (has to get mor grannular control over what is included)
static bool running;

// TODO : Temporary static
static unified_input InputState;
static float zoom_level = 1.0f;


void *PushSize(ScratchArena *arena, size_t size)
{
  uintptr_t current_ptr = (uintptr_t)(arena->base + arena->used);
  //this is just in case  
  uintptr_t offset = current_ptr & 0xF;
  if(offset > 0) {
      offset = 16 - offset;
  }
  
  if (arena->used + offset + size > arena->capacity)
  {
    //TODO : handle out of memory
    Assert(false);
    return nullptr;
  }

  arena->used += offset;
  void *ptr = arena->base + arena->used;
  arena->used += size;

  return ptr;
}

void InitArena(ScratchArena *arena, uint8_t *base, size_t capacity)
{
  arena->base = base;
  arena->capacity = capacity;
  arena->used = 0;
}


static void ResizeDIBSection(HandmadeScreenBuffer *Buffer, BITMAPINFO *BufferInfo, int Width, int Height)
{

  GlobalMemory.Backbuffer.used = 0; // reset backbuffer arena

  Buffer->Width = Width;
  Buffer->Height = Height;
  int BytesPerPixel = 4;

  BufferInfo->bmiHeader.biSize = sizeof(BufferInfo->bmiHeader);
  BufferInfo->bmiHeader.biWidth = Buffer->Width;
  BufferInfo->bmiHeader.biHeight = -Buffer->Height;
  BufferInfo->bmiHeader.biPlanes = 1;
  BufferInfo->bmiHeader.biBitCount = 32;
  BufferInfo->bmiHeader.biCompression = BI_RGB;
  Buffer->Pitch = Buffer->Width * BytesPerPixel;

  int bitMapMemorySize = (Buffer->Width * Buffer->Height) * BytesPerPixel;


  Buffer->Memory = PushSize(&GlobalMemory.Backbuffer, bitMapMemorySize);

  HmadeOnBufferSizeChange(Width, Height);
}

static void Win32CopyBufferToWindow(
    HDC WindowContext, int WindowWidth, int WindowHeight,
    HandmadeScreenBuffer *Buffer, BITMAPINFO *BufferInfo,
    int X, int Y, int Width, int Height)
{

  StretchDIBits(
      WindowContext,
      0, 0, WindowWidth, WindowHeight,
      0, 0, Buffer->Width, Buffer->Height,
      Buffer->Memory,
      BufferInfo,
      DIB_RGB_COLORS, SRCCOPY);
}

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

  mouse_state &MouseState = InputState.Mouse;

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
    Win32CopyBufferToWindow(DeviceContext, window.Width, window.Height, &globalBackBuffer, &globalBackBufferInfo, X, Y, Width, Height);

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
    bool isKeyReleased = (keyFlags & KF_UP) == KF_UP;

    button_state &keyState = InputState.Keyboard.keys[VKCode];
    keyState.ended_down = !isKeyReleased;
    keyState.half_transition_count += 1;

    bool altKeyWasDown = (lParam & (1 << 29)) != 0;

    if (altKeyWasDown && VKCode == VK_F4)
    {
      running = false;
    }
  }
  break;

  case WM_MOUSEMOVE:
  {
    MouseState.x = GET_X_LPARAM(lParam);
    MouseState.y = GET_Y_LPARAM(lParam);
  }
  break;
  case WM_MOUSEWHEEL:
  {
    MouseState.wheel_delta += GET_WHEEL_DELTA_WPARAM(wParam);
  }
  break;

  case WM_LBUTTONDOWN:
    MouseState.onButtonAction(MouseButton_Left, true);
    break;
  case WM_LBUTTONUP:
    MouseState.onButtonAction(MouseButton_Left, false);
    break;
  case WM_RBUTTONDOWN:
    MouseState.onButtonAction(MouseButton_Right, true);
    break;
  case WM_RBUTTONUP:
    MouseState.onButtonAction(MouseButton_Right, false);
    break;
  case WM_MBUTTONDOWN:
    MouseState.onButtonAction(MouseButton_Middle, true);
    break;
  case WM_MBUTTONUP:
    MouseState.onButtonAction(MouseButton_Middle, false);
    break;
  case WM_XBUTTONDOWN:
  case WM_XBUTTONUP:
  {
    MouseState.onButtonAction((HIWORD(wParam) & XBUTTON1) ? MouseButton_X1 : MouseButton_X2, Message == WM_XBUTTONDOWN);
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

  //memory initialization
  GlobalMemory = {};
  GlobalMemory.IsInitialized = false;
  GlobalMemory.TotalSize = GIGABYTES(1); // 1 Gigabyte for the whole game (code + assets + runtime memory)
  GlobalMemory.BasePointer = (uint8_t *)VirtualAlloc(
      nullptr,
      GlobalMemory.TotalSize,
      MEM_RESERVE | MEM_COMMIT,
      PAGE_READWRITE);

  InitArena(&GlobalMemory.Permanent, (uint8_t *)GlobalMemory.BasePointer, MEGABYTES(32)); // 32 MegaBytes for permanent storage
  //colorspace : x*y*(4 RGB bytes per pixel + 4 bytes for depth buffer) -> max 
  InitArena(&GlobalMemory.Backbuffer, (uint8_t *)GlobalMemory.BasePointer + MEGABYTES(32), MEGABYTES(64)); // 64 MegaBytes for backbuffer storage (size for 4K resolution with depth buffer)
  InitArena(&GlobalMemory.Transient, (uint8_t *)GlobalMemory.BasePointer + MEGABYTES(96), GlobalMemory.TotalSize - MEGABYTES(96)); // the rest for transient storage
  
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
  ResizeDIBSection(&globalBackBuffer, &globalBackBufferInfo, windowDim.Width, windowDim.Height);

  // initialize game :
  HandmadeInitialize();

  // audio initialization
  bool hasAudio = false;
  IAudioRenderClient *pRenderAudioClient;
  WAVEFORMATEX *pwfx;

  DWORD audioFlags = 0;
  int wavetime = 0;

  HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  HandmadeSoundOutput SoundStat = {};
  if (SUCCEEDED(hr))
  {
    hr = win32_GetAudioRenderClient(&pRenderAudioClient, &pwfx);

    if (SUCCEEDED(hr))
    {
      SoundStat.SampleRate = pwfx->nSamplesPerSec;
      SoundStat.channels = pwfx->nChannels;
      SoundStat.Frequency = 440.0f;
      SoundStat.Volume = 0.2f;
      SoundStat.SampleIndex = 0;
      //sound buffer never changes size so we can allocate it once here
      SoundStat.Buffer = (float *)PushSize(&GlobalMemory.Permanent, pwfx->nSamplesPerSec * sizeof(float) * SoundStat.channels);
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
    UINT32 framesToFill1 = 512;
    // gets the number of frames to put in the buffer to maintain low latency
    win32GetFramesToFill(*pAudioClient, (UINT32 &)framesToFill1, SoundStat.SampleRate, deltaT);
    uint32_t framesToFill = framesToFill1;

    HDC DeviceContext = GetDC(window);

    HandmadeUpdateAndRender(&globalBackBuffer, &SoundStat, InputState, deltaT, framesToFill);

    Win32WindowDimension Dimension = Win32GetWindowDimension(window);

    Win32CopyBufferToWindow(DeviceContext, Dimension.Width, Dimension.Height, &globalBackBuffer, &globalBackBufferInfo, 0, 0, Dimension.Width, Dimension.Height);
    ReleaseDC(window, DeviceContext);
    // continue audio streaming
    if (hasAudio)
    {
      win32FillAudioBuffer(*pAudioClient, pRenderAudioClient, audioFlags, SoundStat);
    }

    // clears mouse movement delta
    InputState.resetOnEndFrame();

    // advance frame index
    framCount++;

    //clears the transient buffer :
    GlobalMemory.Transient.used = 0;
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

bool os_ReadFile(const char *Filename, void **Dest, unsigned int *FileSize)
{
  HANDLE fileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (fileHandle == INVALID_HANDLE_VALUE)
  {
    OutputDebugStringA("Failed to open file.\n");
    *Dest = nullptr;
    *FileSize = 0;
    return false;
  }

  LARGE_INTEGER fileSizeLI;
  if (!GetFileSizeEx(fileHandle, &fileSizeLI))
  {
    OutputDebugStringA("Failed to get file size.\n");
    CloseHandle(fileHandle);
    *Dest = nullptr;
    *FileSize = 0;
    return false;
  }

  unsigned int fileSize = (unsigned int)fileSizeLI.QuadPart;
  void *buffer = PushSize(&GlobalMemory.Transient, fileSize);
  if (!buffer)
  { //in theory, this should never happen as the transient storage should just crash if we run out of memory
    OutputDebugStringA("Failed to allocate memory for file.\n");
    CloseHandle(fileHandle);
    *Dest = nullptr;
    *FileSize = 0;
    return false;
  }

  DWORD bytesRead;
  if (!ReadFile(fileHandle, buffer, fileSize, &bytesRead, NULL) || bytesRead != fileSize)
  {
    OutputDebugStringA("Failed to read file.\n");
    GlobalMemory.Transient.used -= fileSize; // mark the memory as free in the arena
    CloseHandle(fileHandle);
    *Dest = nullptr;
    *FileSize = 0;
    return false;
  }

  CloseHandle(fileHandle);
  *Dest = buffer;
  *FileSize = fileSize;

  return true;
}

void os_PrintLog(const char *Message)
{
  printf("%s", Message);
}
