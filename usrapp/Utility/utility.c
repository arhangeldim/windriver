#include <Windows.h>
#include <stdio.h>
#include <conio.h>

#include "LpcLib.h"

#define LPC_PORT_NAME L"\\BaseNamedObjects\\ConsoleLpcPort"

int main(int argc, char *argv[])
{
    NTSTATUS    status;
    HANDLE      hPipeOutRd, hPipeInWr;
    
    char    buffer[128];
    DWORD   numBytesRead = 0;
    log("inite: %d\n", GetLastError());

    sscanf(argv[1], "%d", &hPipeOutRd);
    sscanf(argv[2], "%d", &hPipeInWr);

    printf("Out read handle: %d\n In write handle: %d\n", hPipeOutRd, hPipeInWr);
    log("try to read file: %d\n", GetLastError());
    status = ReadFile(
        hPipeOutRd,
        buffer,
        127 * sizeof(char),
        &numBytesRead,
        NULL
        );

    log("CLIENT: readFile status: %d, %d\n", status, GetLastError());

    buffer[numBytesRead] = '\0';
    log("Utility read from pipe: %s\n", buffer);

    getch();
    return 0;
}