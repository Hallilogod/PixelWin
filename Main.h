#include <windows.h>
#include <winsock.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>


#ifdef DEBUG
    #define dbgprintf printf
#else
    #define dbgprintf(...)  
#endif


#include "Canvas.h"
#include "Server.h"






    
    /*blending RGBA with RGB pixel: outputRed = (foregroundRed * foregroundAlpha) + (backgroundRed * (1.0 - foregroundAlpha));*/