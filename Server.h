#pragma once

SOCKET InitializeServerSocket(unsigned short port);

DWORD WINAPI ClientHandlerRoutine(PVOID argument);

void HandleCommand(_Inout_ char *command, UINT commandStrlen, SOCKET clientSocket);