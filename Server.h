#pragma once

SOCKET InitializeServerSocket(unsigned short port);

DWORD WINAPI ClientHandlerRoutine(PVOID argument);