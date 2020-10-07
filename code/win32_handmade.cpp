#include <windows.h>

#define UNUSED(x) (void)(x)

#define internal static
#define local_persist static
#define global_variable static

global_variable bool Running;

global_variable HDC BitmapDeviceContext;
global_variable HBITMAP BitmapHandle;
global_variable void *BitmapMemory;
global_variable BITMAPINFO BitmapInfo;

internal void
Win32ResizeDIBSection(int Width, int Height)
{
    if(BitmapHandle)
    {
        DeleteObject(BitmapHandle);
    }

    if(!BitmapDeviceContext)
    {
        BitmapDeviceContext = CreateCompatibleDC(0);
    }

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = Width;
    BitmapInfo.bmiHeader.biHeight = Height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    BitmapHandle = CreateDIBSection(
        BitmapDeviceContext,
        &BitmapInfo,
        DIB_RGB_COLORS,
        &BitmapMemory,
        0,0);
}

internal void
Win32UpdateWindow(HDC DeviceContext,
                  int X, int Y,
                  int Width, int Height)
{
    StretchDIBits(DeviceContext,
                  X, Y, Width, Height,
                  X, Y, Width, Height,
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

            Win32UpdateWindow(DeviceContext,
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
            Running = true;
            while(Running)
            {
                MSG Message;
                BOOL MessageResult = GetMessageA(&Message, 0, 0, 0);

                if(MessageResult > 0)
                {
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
                else
                {
                    break;
                }
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
