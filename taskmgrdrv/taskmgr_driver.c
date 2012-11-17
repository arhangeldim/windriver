//#include "ntddk.h"
#include "stdio.h"
#include "ntifs.h"
#include <wdm.h>

#define STDCALL __stdcall

#define FILE_DEVICE_TASKMGRDRIVER 0x00008337

#define DBGBRK()

#define debug_

#ifdef debug_
#define DBGBRK() __debugbreak()
#endif 


typedef struct _TASKMGR_DEVICE_EXTENSION {
	PDEVICE_OBJECT	pDeviceObject;
	UNICODE_STRING	usDriverName;
} TASKMGR_DEVICE_EXTENSION, *PTASKMGR_DEVICE_EXTENSION;


#define TASKMGR_DRIVER_NAME L"\\Device\\taskmgrdriver"
#define TASKMGR_DRIVER_NAME_SYMLINK L"\\DosDevices\\dostaskmgrdriver"

enum {
	DRV_INIT = 0x801,
};


#define IOCTL_TASKMGR_INIT \
	CTL_CODE(FILE_DEVICE_UNKNOWN, \
			DRV_INIT, \
			METHOD_BUFFERED, \
			FILE_ANY_ACCESS)
	


// Forward decl
NTSTATUS STDCALL DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pusRegPath);
NTSTATUS STDCALL IoCtlTaskmgr(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);

VOID STDCALL CmdUnload(PDRIVER_OBJECT pDriverObject);

NTSTATUS CompleteIrp(PIRP Irp, NTSTATUS status, ULONG info)
{
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = info;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS CloseHandleIRPHandler(IN PDEVICE_OBJECT pDo, IN PIRP Irp)
{
	DbgPrint("[tskmgrdrv] CloseHandler\n");
	
	return CompleteIrp(Irp, STATUS_SUCCESS, 0);
}

NTSTATUS ReadWriteIRPhandler(IN PDEVICE_OBJECT pDo, IN PIRP Irp )
{
	DbgPrint("[tskmgrdrv] ReadWriteIRPHandler\n");
	return CompleteIrp(Irp, STATUS_SUCCESS, 0);
}




VOID WorkerThreadRoutine(PVOID parameter)
{
	
}


NTSTATUS HandleTaskmgrInit(PIRP pIrp, PIO_STACK_LOCATION pIoStackLocation, ULONG *pdwDataWritten)
{
	NTSTATUS	status = 0;
	PCHAR		pInBuffer, pOutBuffer;
	PULONG		handles;
	ULONG		dwDataRead, dwDataWritten;
	HANDLE		hPipeOutWr = (HANDLE) -1;
	HANDLE		hPipeInRd = (HANDLE) -1;
	FILE_OBJECT	file;
	HANDLE		hKePipeOutWr;
		
	
	/* TODO: rewrite this */
	PCHAR pReturnData = "IOCTL - Direct Out I/O From Kernel!";
    ULONG dwDataSize = sizeof("IOCTL - Direct Out I/O From Kernel!");
	
	memset(&file, 0, sizeof(file));
	
	pInBuffer = pIrp->AssociatedIrp.SystemBuffer;
	pOutBuffer = pIrp->AssociatedIrp.SystemBuffer;
	
	handles = (PULONG) pInBuffer;
	DBGBRK();
	if (pInBuffer && pOutBuffer) {
		
		hPipeOutWr = (HANDLE) handles[0];
		hPipeInRd = (HANDLE) handles[1];
		
		
		DBGBRK();
		
		
		
		status = ObReferenceObjectByHandle(
			hPipeOutWr,
			0,
			*IoFileObjectType,
			UserMode,
			(PVOID) &file,
			NULL
			);
		DBGBRK();
		DbgPrint("ObReference: 0x%x\n", status);
		
		if (!NT_SUCCESS(status))
			return status;
		
		// 0n-1073741788
	    /* STATUS = 0xc0000024 - STATUS_OBJECT_TYPE_MISMATCH */
 		status = ObOpenObjectByPointer(
			&file,
			OBJ_KERNEL_HANDLE,
			NULL,
			0,
			*IoFileObjectType, // -> NULL
			UserMode,  // -> KernelMode
			&hKePipeOutWr
			);
		
		DbgPrint("ObOpenObjectByPointer: 0x%x\n", status);
		
	}


	
	return status;
}


NTSTATUS CreateFileIRPHandler(IN PDEVICE_OBJECT pDo, IN PIRP pIrp)
{
	
	NTSTATUS	status;
	PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	DbgPrint("[tskmgrdrv] CreateFile %ws\n", pIrpStack->FileObject->FileName.Buffer);

	return CompleteIrp(pIrp, STATUS_SUCCESS, 0);

}
		
#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text(PAGE, IoCtlTaskmgr)
#pragma alloc_text(PAGE, CmdUnload)

#endif		


NTSTATUS STDCALL DriverEntry(
	PDRIVER_OBJECT pDriverObject,
	PUNICODE_STRING pusRegPath
	)
{
	UNICODE_STRING	usNameString, usLinkString;
	
	PDEVICE_OBJECT	device;
	NTSTATUS		status;

	status = STATUS_SUCCESS;
	
	
	
	
	DBGBRK();
	
		
	pDriverObject->DriverUnload = CmdUnload;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoCtlTaskmgr;
	pDriverObject->MajorFunction[IRP_MJ_CREATE]= CreateFileIRPHandler;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseHandleIRPHandler;
	pDriverObject->MajorFunction[IRP_MJ_READ]  = ReadWriteIRPhandler;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = ReadWriteIRPhandler;
	
	
	RtlInitUnicodeString(&usNameString, TASKMGR_DRIVER_NAME);
	RtlInitUnicodeString(&usLinkString, TASKMGR_DRIVER_NAME_SYMLINK);
	DbgPrint("DriverEntry started\n");
	
	status = IoCreateDevice(
		pDriverObject,
		0,
		&usNameString,
		FILE_DEVICE_UNKNOWN,
		0,
		TRUE,
		&device
		);
	
	if( !NT_SUCCESS(status) ) {
        DbgPrint(("TaskmgrDriver: Failed to create device\n"));
        RtlFreeUnicodeString( &usNameString );
        return status;
    }

	DbgPrint("Device created: %x\n", status);
	
	
	DBGBRK();
	
	status = IoCreateSymbolicLink(&usLinkString, &usNameString);
	if (!NT_SUCCESS(status))
		DbgPrint("Failed create link status=%x\n", status);
		
	DbgPrint("Create link status = %x\n", status);	
	

	
	return status;	
}


VOID CmdUnload(PDRIVER_OBJECT pDriverObject)
{
	IoDeleteDevice(pDriverObject->DeviceObject);
}

NTSTATUS STDCALL IoCtlTaskmgr(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	NTSTATUS	status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG		dwDataWritten = 0;
	DBGBRK();
	DbgPrint("IOCTL read\n");
	
	if (pIrpStack) {
		switch(pIrpStack->Parameters.DeviceIoControl.IoControlCode) {
			case IOCTL_TASKMGR_INIT:
				DbgPrint("Handle TASKMGR_INIT ioctl\n");
				status = HandleTaskmgrInit(pIrp, pIrpStack, &dwDataWritten);
				break;
			default:
				break;
		}
	}
	
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = dwDataWritten;
	
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	
	return status;
}