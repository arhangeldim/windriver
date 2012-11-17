#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include "LpcLib.h"

#define LPC_PORT_NAME L"\\BaseNamedObjects\\ConsoleLpcPort"
#define TASKMGR_DRIVER_NAME L"\\\\.\\dostaskmgrdriver"

enum {
	DRV_INIT = 0x801,
};
	
#define IOCTL_TASKMGR_INIT \
	CTL_CODE(FILE_DEVICE_UNKNOWN, DRV_INIT, METHOD_BUFFERED, FILE_ANY_ACCESS)
	

#define BUF_SIZE 32

BOOL SendDeviceIoControl(LPCWSTR deviceName, DWORD *handles, DWORD hbSize)
{
    BOOL    bResult;
    HANDLE  hDevice;
    CHAR    pOut[BUF_SIZE];
    DWORD   bytesWritten = 0;

    DWORD   mData[2];
    mData[0] = 789;
    mData[1] = 456;

   
    log("Creating device %S\t", deviceName);
    hDevice = CreateFile(
             deviceName,
             GENERIC_READ | GENERIC_WRITE, 0, NULL,
             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        log("FAILED: 0x%x\n", GetLastError());
        return FALSE;
    } else {
        log("SUCCESS\n");
    }

    ZeroMemory(pOut, BUF_SIZE);

    bResult = DeviceIoControl(
        hDevice,
        IOCTL_TASKMGR_INIT,
        handles,
        hbSize,
        pOut,
        BUF_SIZE,
        &bytesWritten,
        (LPOVERLAPPED) NULL
        );
    if (!bResult) {
        log("FAILED: 0x%x\n", GetLastError());
        return FALSE;
    } else {
        log("SUCCESS\n");
        log("Bytes written: %d\n", bytesWritten);
    }
    CloseHandle(hDevice);
    return TRUE;
}

int main(void)
{
    HANDLE  hChildProcess;
    HANDLE  hPipeOutRd, hPipeOutWr;
    HANDLE  hPipeInRd, hPipeInWr;
    HANDLE  hPipeOutRdDup, hPipeInWrDup;
    HANDLE  hPipeOutWrDup, hPipeInRdDup;
    BOOL    status;
    PROCESS_INFORMATION pi;
    STARTUPINFO si = {sizeof(si)};

    SERVER_LPC_PORT lpcPort;
    NTSTATUS        ntStatus;

    BYTE bHandles[16];
    WORD bLength;

    DWORD   dwHandles[2];

    char   handleToDriver[16];
    char    tmpWrHandle[4] = {'\0'};
    int     htdSize = 0;

    wchar_t pCmdLine[128] = {'\0'};
    wchar_t pTmpRdHandle[4] = {'\0'};
    wchar_t pTmpWrHandle[4] = {'\0'};

    char toPipe[] = "send to pipe!";
    DWORD   numBytesWritten;

    long statVal = -1073741788;
   
    /* Create pipe for OUT */
    status = CreatePipe(&hPipeOutRd, &hPipeOutWr, NULL, 0);
    if (!status) {
        fprintf(stderr, "Failed to create OUT pipe.\n");
        return 1;
    }

     /* For IN */
    status = CreatePipe(&hPipeInRd, &hPipeInWr, NULL, 0);
    if (!status) {
        fprintf(stderr, "Failed to create IN pipe.\n");
        return 1;
    }

    status = DuplicateHandle(GetCurrentProcess(), hPipeOutRd, GetCurrentProcess(), &hPipeOutRdDup, 0, TRUE, DUPLICATE_SAME_ACCESS);
    CloseHandle(hPipeOutRd);
    status = DuplicateHandle(GetCurrentProcess(), hPipeInWr, GetCurrentProcess(), &hPipeInWrDup, 0, TRUE, DUPLICATE_SAME_ACCESS);
    CloseHandle(hPipeInWr);

    //status = DuplicateHandle(GetCurrentProcess(), hPipeOutWr, GetCurrentProcess(), &hPipeOutWrDup, 0, TRUE, DUPLICATE_SAME_ACCESS);
    //CloseHandle(hPipeOutWr);
    //status = DuplicateHandle(GetCurrentProcess(), hPipeInRd, GetCurrentProcess(), &hPipeInRdDup, 0, TRUE, DUPLICATE_SAME_ACCESS);
    //CloseHandle(hPipeInRd);

    /* Save handles for driver */

    memcpy(handleToDriver, &hPipeOutWr, sizeof(hPipeOutWr));
    
    htdSize = sizeof(hPipeOutWr);
    dwHandles[0] = (DWORD) hPipeOutWr;
    dwHandles[1] = (DWORD) hPipeInRd;
    printf("FOR IOCTL: %d\t%d\n", dwHandles[0], dwHandles[1]);


    wcscat(pCmdLine, L"utility");
    wcscat(pCmdLine, L" ");
    wsprintf(pTmpRdHandle, L"%d", hPipeOutRdDup);
    wcscat(pCmdLine, pTmpRdHandle);
    wcscat(pCmdLine, L" ");
    wsprintf(pTmpWrHandle, L"%d", hPipeInWrDup);
    wcscat(pCmdLine, pTmpWrHandle);

    printf("SERVER set cmdLine: %S\n", pCmdLine);

    /* Create utility process */
    printf("Creating new process...");
    status = CreateProcess(
        TEXT(".\\utility.exe"),
        pCmdLine,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &si,
        &pi
        );

    printf("SERVER: %x\n", GetLastError());
    hChildProcess = pi.hProcess;
    printf("INFO: %d\n", hChildProcess);
    if (!status) {
        DWORD err = GetLastError();
        printf("Failed: 0x%x\t%d\n", err, err);
        return 1;
    }


    SendDeviceIoControl(TASKMGR_DRIVER_NAME, dwHandles, 2 * sizeof(DWORD));

	
	
    printf("Send status : %d\n", status);
    getch();
    return 0;
}



    

