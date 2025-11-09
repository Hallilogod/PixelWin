#include "Main.h"


int main(void)
{
    int argc = 0;
    PWSTR* wargv = NULL; 
    unsigned short serverPort = 0;
    UINT canvasLength = 0;
    UINT canvasHeight = 0;
    WSADATA wsaStartupData = { 0 };


   LPWSTR commandLine = GetCommandLineW();

    wargv = CommandLineToArgvW(commandLine, &argc);

    if(argc < 4)
    {
        printf("Usage: %ls <port> <xSize> <ySize>", *wargv);

        return EXIT_SUCCESS;
    }

    serverPort = (unsigned short)wcstoul(wargv[1], NULL, 10);
    canvasLength = (unsigned int)wcstoul(wargv[2], NULL, 10);
    canvasHeight = (unsigned int)wcstoul(wargv[3], NULL, 10);

    if(serverPort == 0)
    {
        printf("Invalid server port '%ls'", wargv[1]);

        return EXIT_FAILURE;
    }

    if(canvasLength == 0)
    {
        printf("Invalid canvas length '%ls'", wargv[2]);

        return EXIT_FAILURE;
    }

    if(canvasHeight == 0)
    {
        printf("Invalid canvas height '%ls'", wargv[3]);

        return EXIT_FAILURE;
    }


    if(WSAStartup(MAKEWORD(2, 2), &wsaStartupData) != 0)
    {
        printf("Failed to start WSA subsystem, WSAError: %d", WSAGetLastError());

        return EXIT_FAILURE;
    }

    SetProcessDPIAware();
    
    InitializeServerCanvas(canvasLength, canvasHeight);

    SOCKET serverSocket = InitializeServerSocket(serverPort);

    if(serverSocket == INVALID_SOCKET)
    {
        printf("Failed to initialize server socket");

        return EXIT_FAILURE;
    }

    printf("Listening for incoming connections...\n");

    while(1)
    {  
        SOCKADDR_IN clientAddress = { 0 };
        int clientAddrLen = sizeof(clientAddress);
        DWORD tid = 0;

        SOCKET clientSocket = accept(serverSocket, (PSOCKADDR)&clientAddress, &clientAddrLen);

        if(clientSocket == INVALID_SOCKET)
        {
            dbgprintf("Failed to accept client connection\n");
            continue;
        }

        printf("Handling incoming connection from client %s\n", inet_ntoa(clientAddress.sin_addr));

        HANDLE hThread = CreateThread(NULL, 0, ClientHandlerRoutine, (PVOID)clientSocket, 0, &tid);

        if(hThread == INVALID_HANDLE_VALUE)
        {
            printf("Error while creating client handler thread, lasterror: %lu", GetLastError());

            closesocket(clientSocket);
        }

        CloseHandle(hThread);
    }


    WSACleanup();

    

    return EXIT_SUCCESS;
}