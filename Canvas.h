#pragma once

typedef struct {
    LONG CanvasWidth;
    LONG CanvasHeight;
} CANVAS_DIMENSIONS;


// Since the RGB macro from wingdi is actually BGR we name ours RGB_RGB, thanks microsoft...
#define RGB_RGB(r, g, b) ((COLORREF)(((BYTE)(b) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(r)) << 16)))

#define RGBA_R(rgba) (((rgba) >> 24) & 0xFF)
#define RGBA_G(rgba) (((rgba) >> 16) & 0xFF)
#define RGBA_B(rgba) (((rgba) >> 8) & 0xFF)
#define RGBA_A(rgba) ((rgba) & 0xFF)

#define RGB_R(rgb) (((rgb) >> 16) & 0xFF)
#define RGB_G(rgb) (((rgb) >> 8) & 0xFF)
#define RGB_B(rgb) ((rgb) & 0xFF)

void UpdateCanvasWindow(void);

void GetCanvasDimensions(_Out_ CANVAS_DIMENSIONS* pCanvasDimensions);

BOOL InitializeServerCanvas(LONG width, LONG height);

void CanvasDrawPixel(LONG x, LONG y, UINT32 rgb);

void CanvasDrawPixelAlpha(LONG x, LONG y, UINT32 rgba);

UINT32 CanvasGetPixel(LONG x, LONG y);


