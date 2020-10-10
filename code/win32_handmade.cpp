#include <windows.h>
#include <stdlib.h>
#include <stdint.h>

#define UNUSED(x) (void)(x)

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

global_variable bool Running;

global_variable void *BitmapMemory;
global_variable BITMAPINFO BitmapInfo;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

internal void
RenderWeirdGradient(int XOffset, int YOffset)
{
    int Width = BitmapWidth;

    int Pitch = Width * BytesPerPixel;
    uint8 *Row = (uint8 *) BitmapMemory;
    for(int Y = 0; Y < BitmapHeight; Y++)
    {
        uint32 *Pixel = (uint32 *) Row;
        for(int X = 0; X < BitmapWidth; X++)
        {
            uint8 Blue = (X + XOffset);
            uint8 Green = (Y + YOffset);
            *Pixel++ = Green << 8 | Blue;
        }

        Row += Pitch;
    }
}

internal void
Win32ResizeDIBSection(int Width, int Height)
{
    BitmapWidth = Width;
    BitmapHeight = Height;

    if(BitmapMemory)
    {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = BytesPerPixel * BitmapWidth * BitmapHeight;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32UpdateWindow(HDC DeviceContext,
                  RECT *ClientRect, 
                  int X, int Y,
                  int Width, int Height)
{
    int WindowWidth = ClientRect -> right - ClientRect -> left;
    int WindowHeight = ClientRect -> bottom - ClientRect -> top;
    StretchDIBits(DeviceContext,
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, BitmapWidth, BitmapHeight,
                  BitmapMemory,
                  &BitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}

LRESULT CALLBACK
Win32WindowProc(HWND   hwnd,
  UINT   uMsg,
  WPARAM wParam,
  LPARAM lParam)
{
    LRESULT Result = 0;
    switch(uMsg)
    {
        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(hwnd, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;
            Win32ResizeDIBSection(Width, Height);
        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext =  BeginPaint(
                hwnd,
                &Paint);

            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            RECT ClientRect;
            GetClientRect(hwnd, &ClientRect);
            Win32UpdateWindow(DeviceContext,
                              &ClientRect,
                              X, Y,
                              Width, Height);

            EndPaint(hwnd, &Paint);
        } break;

        case WM_CLOSE:
        {
            Running = false;
            OutputDebugStringA("WM_CLOSE\n");
        } break;

        case WM_DESTROY:
        {
            Running = false;
            OutputDebugStringA("WM_DESTROY\n");
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        default:
        {
            Result = DefWindowProcA(hwnd, uMsg, wParam, lParam);
        } break;
    }

    return Result;
}

int CALLBACK WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nShowCmd)
{
    UNUSED(hInstance);
    UNUSED(hPrevInstance);
    UNUSED(lpCmdLine);
    UNUSED(nShowCmd);

    WNDCLASSA WindowClass = {0};
    WindowClass.hInstance = hInstance;
    WindowClass.lpfnWndProc = Win32WindowProc;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass"; 

    if(RegisterClassA(&WindowClass))
    {
        HWND Window = CreateWindowExA(0,
                        WindowClass.lpszClassName,
                        "Handmade Hero",
                        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        0,0,hInstance,0);
        if(Window)
        {
            int XOffset = 0;
            int YOffset = 0;

            Running = true;

            while(Running)
            {
                MSG Message;

                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        Running = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
                RenderWeirdGradient(XOffset, YOffset);

                HDC DeviceContext = GetDC(Window);
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                Win32UpdateWindow(DeviceContext,
                    &ClientRect,
                    0, 0,
                    WindowWidth, WindowHeight);
                ReleaseDC(Window, DeviceContext);

                XOffset++;
            }
        }
        else
        {
            //TODO: Logging
        }
        
    }
    else
    {
        //TODO: Logging
    }

    return(0);
}
