#include "Main.h"

static PVOID pixelCanvasBuffer = NULL;
static HDC canvasDC = NULL;
static HDC canvasMemDC = NULL;
static volatile LONG canvasWidth = 0;
static volatile LONG canvasHeight = 0;

void UpdateCanvasWindow(void)
{
    BitBlt(canvasDC, 0, 0, canvasWidth, canvasHeight, canvasMemDC, 0, 0, SRCCOPY);
}

UINT32 CanvasGetPixel(LONG x, LONG y)
{
    if ((x >= canvasWidth) || (y >= canvasHeight) || (y < 0) || (x < 0))
    {
        return 0;
    }

    return ((PUINT32)pixelCanvasBuffer)[y * canvasWidth + x];
}

void GetCanvasDimensions(_Out_ CANVAS_DIMENSIONS *pCanvasDimensions)
{
    pCanvasDimensions->CanvasHeight = canvasHeight;
    pCanvasDimensions->CanvasWidth = canvasWidth;
}


void CanvasDrawPixel(LONG x, LONG y, UINT32 rgb)
{
    if ((x >= canvasWidth) || (y >= canvasHeight) || (y < 0) || (x < 0))
    {
        return;
    }

    InterlockedExchange(
        (LONG volatile *)(((LONG volatile *)pixelCanvasBuffer) + (y * canvasWidth + x)),
        RGB_RGB((rgb & 0xFF0000) >> 16, (rgb & 0x00FF00) >> 8, rgb & 0xFF));
}

void CanvasDrawPixelAlpha(LONG x, LONG y, UINT32 rgba)
{
    if ((x >= canvasWidth) || (y >= canvasHeight) || (y < 0) || (x < 0))
    {
        return;
    }

    double alphaA = (double)(rgba & 0xFF) / 255;

    UINT32 canvasPixel = ((PUINT32)pixelCanvasBuffer)[y * canvasWidth + x];

    // Perform alpha composition
    UINT8 r = alphaA * RGBA_R(rgba) + (1 - alphaA) * RGB_R(canvasPixel);
    UINT8 g = alphaA * RGBA_G(rgba) + (1 - alphaA) * RGB_G(canvasPixel);
    UINT8 b = alphaA * RGBA_B(rgba) + (1 - alphaA) * RGB_B(canvasPixel);

    InterlockedExchange((LONG volatile *)(((LONG volatile *)pixelCanvasBuffer) + (y * canvasWidth + x)), RGB_RGB(r, g, b));
}

static LRESULT CALLBACK CanvasWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        ExitProcess(0);
        return 0;
        break;

    case WM_ENTERSIZEMOVE:
        return 0;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

static DWORD CanvasThreadProc(PVOID argument)
{

    DWORD windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE;
    CANVAS_DIMENSIONS *pCanvasArgs = (CANVAS_DIMENSIONS *)argument;

    HBITMAP dibSection = NULL;
    BITMAPINFO canvasBitmapInfo;

    InterlockedExchange(&canvasWidth, pCanvasArgs->CanvasWidth);
    InterlockedExchange(&canvasHeight, pCanvasArgs->CanvasHeight);

    RtlZeroMemory(&canvasBitmapInfo, sizeof(canvasBitmapInfo));

    canvasBitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    canvasBitmapInfo.bmiHeader.biWidth = canvasWidth;
    canvasBitmapInfo.bmiHeader.biHeight = -canvasHeight;
    canvasBitmapInfo.bmiHeader.biPlanes = 1;
    canvasBitmapInfo.bmiHeader.biBitCount = 32;
    canvasBitmapInfo.bmiHeader.biCompression = BI_RGB;

    HWND canvasWndHandle = CreateWindowExA(
        0,
        "MainClass",
        "Pixelflut!",
        windowStyle,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        pCanvasArgs->CanvasWidth,
        pCanvasArgs->CanvasHeight,
        NULL,
        NULL,
        NULL,
        NULL);

    if (canvasWndHandle == NULL)
    {
        printf("Error creating canvas window, lasterror: %lu\n", GetLastError());
    }

    canvasDC = GetDC(canvasWndHandle);

    dibSection = CreateDIBSection(canvasDC, &canvasBitmapInfo, DIB_RGB_COLORS, &pixelCanvasBuffer, NULL, 0);

    canvasMemDC = CreateCompatibleDC(canvasDC);

    SelectObject(canvasMemDC, dibSection);

    MSG windowMsg = {0};

    while (1)
    {
        UpdateCanvasWindow();

        if (PeekMessageA(&windowMsg, canvasWndHandle, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&windowMsg);
            DispatchMessageA(&windowMsg);
        }

        Sleep(5);
    }

    return EXIT_SUCCESS;
}

BOOL InitializeServerCanvas(LONG width, LONG height)
{
    DWORD tid = 0;
    CANVAS_DIMENSIONS *pCanvasArguments = malloc(sizeof(CANVAS_DIMENSIONS));
    if (pCanvasArguments == NULL)
    {
        printf("Failed to allocate buffer for canvas thread arguments\n");

        return FALSE;
    }

    pCanvasArguments->CanvasWidth = width;
    pCanvasArguments->CanvasHeight = height;

    WNDCLASSA windowClass = {0};
    windowClass.lpfnWndProc = CanvasWindowProc;
    windowClass.lpszClassName = "MainClass";

    GetModuleHandleExA(0, NULL, &windowClass.hInstance);
    RegisterClassA(&windowClass);

    HANDLE canvasThread = CreateThread(NULL, 0, CanvasThreadProc, pCanvasArguments, 0, &tid);

    if (canvasThread == INVALID_HANDLE_VALUE)
    {
        printf("Failed to create canvas thread\n");

        free(pCanvasArguments);
        return FALSE;
    }

    dbgprintf("Canvas thread %lu created\n", tid);

    CloseHandle(canvasThread);

    return TRUE;
}