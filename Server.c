#include "Main.h"

SOCKET InitializeServerSocket(unsigned short port)
{
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET)
    {
        printf("Failed to initialize server socket\n");

        return INVALID_SOCKET;
    }

    SOCKADDR_IN socketAddress = {0};

    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = INADDR_ANY;
    socketAddress.sin_port = htons(port);

    if (bind(serverSocket, (PSOCKADDR)&socketAddress, sizeof(socketAddress)))
    {
        printf("Error binding server socket, wsaerror: %d\n", WSAGetLastError());

        closesocket(serverSocket);

        return INVALID_SOCKET;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("Error: listen failed with WSAError %d\n", WSAGetLastError());

        closesocket(serverSocket);

        return INVALID_SOCKET;
    }

    return serverSocket;
}

static UINT64 StrtolDuff(char* s, UINT strLen)
{
    UINT64 val = 0;
    int n = (strLen + 7) / 8;
    int i = 0;
    switch (strLen % 8) {
    case 0: do {
                 val = val * 10 + (*(s + i++) - '0');
    case 7:      val = val * 10 + (*(s + i++) - '0');
    case 6:      val = val * 10 + (*(s + i++) - '0');
    case 5:      val = val * 10 + (*(s + i++) - '0');
    case 4:      val = val * 10 + (*(s + i++) - '0');
    case 3:      val = val * 10 + (*(s + i++) - '0');
    case 2:      val = val * 10 + (*(s + i++) - '0');
    case 1:      val = val * 10 + (*(s + i++) - '0');
    } while (--n > 0);
    }
    return val;
}


void HandleCommand(_Inout_ char *command, UINT commandStrlen, SOCKET clientSocket)
{
    if (strncmp(command, "PX ", 3) == 0)
    {   
        UINT x = 0;
        UINT y = 0;
        UINT32 rgb_a = 0; 

        char* subCommand = command + 3;
        char* subCommandPost = subCommand;
        char* commandEnd = command + commandStrlen;
        char c;
        
        for(;;)
        {
            c = *subCommandPost;

            if(c == ' ')
            {
                break;
            }

            if(c == '\0')
            {
                return;
            }

            subCommandPost++;
        }

        x = (UINT)StrtolDuff(subCommand, subCommandPost - subCommand);

        subCommandPost++;

        subCommand = subCommandPost;

        if(*subCommand == '\0')
        {
            return;
        }

        for(;;)
        {
            c = *subCommandPost;

            if(c == ' ')
            {
                break;
            }

            if(c == '\0')
            {
                break;
            }

            subCommandPost++;
        }

        y = (UINT)StrtolDuff(subCommand, subCommandPost - subCommand);

        // If the command ends after the PX X Y then its asking for the pixel color
        if(commandEnd == subCommandPost)
        {   
            // Six for rrggbb, one for space, one for line feed, one for null terminator
            char rgbTxtBuffer[9];

            sprintf_s(rgbTxtBuffer, sizeof(rgbTxtBuffer), " %06x\n", CanvasGetPixel(x, y));

            // Since we respond with the same PX X Y but with rrggbb after it, we can first send back the command we got
            send(clientSocket, command, commandStrlen, 0);

            // Then, send the pixel color
            send(clientSocket, rgbTxtBuffer, sizeof(rgbTxtBuffer) - 1 /* Don't send null byte */, 0);

            return;
        }

        subCommandPost++;

        subCommand = subCommandPost;

        rgb_a = strtoul(subCommand, NULL, 16);
        size_t substringLength = commandEnd - subCommand;

        switch(substringLength)
        {
            case 6:
            {
                CanvasDrawPixel(x, y, rgb_a);
            }
            break;

            case 8:
            {
                CanvasDrawPixelAlpha(x, y, rgb_a);
            }
            break;

            default:
                return;
            break;
        }

    }
    else if (strncmp(command, "SIZE", 4) == 0)
    {
        char sizeStrBuffer[64];
        CANVAS_DIMENSIONS canvasDimensions = {0};

        GetCanvasDimensions(&canvasDimensions);

        sprintf_s(sizeStrBuffer, sizeof(sizeStrBuffer), "SIZE %ld %ld\n", canvasDimensions.CanvasWidth, canvasDimensions.CanvasHeight);

        send(clientSocket, sizeStrBuffer, strlen(sizeStrBuffer), 0);
    }
    else if (strncmp(command, "HELP", 4) == 0)
    {
        char *helpText = "This is pixelflut, draw something by sending 'PX x y rrggbb(aa)'! Read more at https://github.com/defnull/pixelflut\n";
        send(clientSocket, helpText, strlen(helpText), 0);
    }
}

DWORD WINAPI ClientHandlerRoutine(PVOID argument)
{
    char dataBuffer[16384] = {0};
    char commandBuffer[32 + 1] = {0};
    int recvResult = 0;
    BOOL waitForNextNewline = FALSE;
    UINT commandBufferIndex = 0;
    SOCKET clientSocket = (SOCKET)argument;
    DWORD currentTid = GetCurrentThreadId();
#ifdef DEBUG
    UINT64 volatile commandHandlerRoutineCycles = 0;
    UINT volatile commandHandlerCalls = 0;
#endif

    while ((recvResult = recv(clientSocket, dataBuffer, sizeof(dataBuffer), 0)) > 0)
    {
        for (int i = 0; i < recvResult; i++)
        {
            if (waitForNextNewline)
            {
                if (dataBuffer[i] == '\n')
                {
                    waitForNextNewline = FALSE;
                }

                continue;
            }

            if (commandBufferIndex == (sizeof(commandBuffer) - 1))
            {
                dbgprintf("[Thread %lu]: Recieved command too big!\n", currentTid);
                commandBufferIndex = 0;

                /* If the current char is a newline then the next char is already
                the next command so we dont need to wait for a newline */
                if (dataBuffer[i] != '\n')
                {
                    waitForNextNewline = TRUE;
                }

                continue;
            }

            char currentBufferChar = dataBuffer[i];

            if(currentBufferChar == '\r')
            {
                continue;
            }

            if (currentBufferChar == '\n')
            {
                commandBuffer[commandBufferIndex] = '\0';
                
                #ifdef DEBUG
                    UINT64 volatile tsc = _rdtsc();
                #endif
                HandleCommand(commandBuffer, commandBufferIndex, clientSocket);

                #ifdef DEBUG
                    commandHandlerRoutineCycles += _rdtsc() - tsc;

                    commandHandlerCalls++;
                #endif
                
                commandBufferIndex = 0;
                continue;
            }

            commandBuffer[commandBufferIndex++] = currentBufferChar;
        }
    }

#ifdef DEBUG
    if(commandHandlerCalls > 0)
    {
        dbgprintf("Command handling routine avg cycles: %llu\n", (UINT64)(commandHandlerRoutineCycles / commandHandlerCalls));
    }
#endif

    closesocket(clientSocket);

    if (recvResult == SOCKET_ERROR)
    {
        dbgprintf("[Thread %lu]: Failed to recv client data, WSAError: %d\n", currentTid, WSAGetLastError());
        return EXIT_FAILURE;
    }
    else
    {
        dbgprintf("[Thread %lu]: Client connection closed\n", currentTid);
    }

    return EXIT_SUCCESS;
}
