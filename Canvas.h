#pragma once

typedef struct {
    LONG CanvasWidth;
    LONG CanvasHeight;
} CANVAS_DIMENSIONS;



#define RGB_LE(r, g, b) ((COLORREF)(((BYTE)(b) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(r)) << 16)))

#define RGB_R(rgb) ((rgb) & 0xFF)
#define RGB_G(rgb) ((rgb >> 8) & 0xFF)
#define RGB_B(rgb) ((rgb >> 16) & 0xFF)

#define RGB_LE_R(rgb) ((rgb >> 16) & 0xFF)
#define RGB_LE_G(rgb) ((rgb >> 8) & 0xFF)
#define RGB_LE_B(rgb) ((rgb) & 0xFF)


void GetCanvasDimensions(_Out_ CANVAS_DIMENSIONS* pCanvasDimensions);

BOOL InitializeServerCanvas(LONG width, LONG height);

void CanvasDrawPixel(LONG x, LONG y, UINT32 rgb_le);