#include <windows.h>
#include <winsock.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>


#ifdef DEBUG
    #define dbgprintf printf
#else
    #define dbgprintf(...)  
#endif


#include "Canvas.h"
#include "Server.h"
