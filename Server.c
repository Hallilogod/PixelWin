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

void HandleCommand(char *command, SOCKET clientSocket)
{
    UINT x = 0;
    UINT y = 0;
    UINT32 rgb = 0;

    if (sscanf_s(command, "PX %u %u %08x", &x, &y, &rgb) == 3)
    {
        CanvasDrawPixel(x, y, rgb);
    }
    else if (strncmp(command, "SIZE", 4) == 0)
    {
        char sizeStrBuffer[64];
        CANVAS_DIMENSIONS canvasDimensions = { 0 };

        GetCanvasDimensions(&canvasDimensions);

        sprintf_s(sizeStrBuffer, sizeof(sizeStrBuffer), "SIZE %ld %ld\n", canvasDimensions.CanvasWidth, canvasDimensions.CanvasHeight);

        send(clientSocket, sizeStrBuffer, strlen(sizeStrBuffer), 0);
    }
    else if(strncmp(command, "HELP", 4) == 0)
    {
        char* helpText = "This is pixelflut, draw something by sending 'PX x y rrggbb(aa)'! Read more at https://github.com/defnull/pixelflut\n";
        send(clientSocket, helpText, strlen(helpText), 0);
    }
}


DWORD WINAPI ClientHandlerRoutine(PVOID argument)
{
    char dataBuffer[16384] = {0};
    char commandBuffer[32 + 1] = {0};
    int recvResult = 0;
    BOOL waitForNextNewline = FALSE;
    int commandBufferIndex = 0;
    SOCKET clientSocket = (SOCKET)argument;
    DWORD currentTid = GetCurrentThreadId();

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
                the next command so wer dont need to wait for a newline */
                if (dataBuffer[i] != '\n')
                {
                    waitForNextNewline = TRUE;
                }

                continue;
            }

            if (dataBuffer[i] == '\n')
            {
                commandBuffer[commandBufferIndex] = '\0';

                commandBufferIndex = 0;

                dbgprintf("Handling command %s\n", commandBuffer);

                HandleCommand(commandBuffer, clientSocket);
                continue;
            }

            commandBuffer[commandBufferIndex++] = dataBuffer[i];
        }
    }

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
