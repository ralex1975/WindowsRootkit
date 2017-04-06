/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.


Module Name:

    testapp.c

Abstract:

    Purpose of this app to test the NONPNP sample driver. The app
    makes four different ioctl calls to test all the buffer types, write
    some random buffer content to a file created by the driver in \SystemRoot\Temp
    directory, and reads the same file and matches the content.
    If -l option is specified, it does the write and read operation in a loop
    until the app is terminated by pressing ^C.

    Make sure you have the \SystemRoot\Temp directory exists before you run the test.

Environment:

    Win32 console application.

--*/

        
#include <DriverSpecs.h>
_Analysis_mode_(_Analysis_code_type_user_code_)  

#include <windows.h>

#pragma warning(disable:4201)  // nameless struct/union
#include <winioctl.h>
#pragma warning(default:4201)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <strsafe.h>
#include "public.h"
#define TESTAPP_C
#include "testapp.h"
#undef TESTAPP_C

VOID ReadIDT();
VOID ReadFunc();

BOOLEAN
ManageDriver(
    IN LPCTSTR  DriverName,
    IN LPCTSTR  ServiceName,
    IN USHORT   Function
    );

HMODULE
LoadWdfCoInstaller(
    VOID
    );

VOID
UnloadWdfCoInstaller(
    HMODULE Library
    );

BOOLEAN
SetupDriverName(
    _Inout_updates_all_(BufferLength) PCHAR DriverLocation,
    _In_ ULONG BufferLength
    );

VOID
DoIoctls(
    HANDLE hDevice
    );

// for example, WDF 1.9 is "01009". the size 6 includes the ending NULL marker
//
#define MAX_VERSION_SIZE 6

CHAR G_coInstallerVersion[MAX_VERSION_SIZE] = {0};
BOOLEAN  G_fLoop = FALSE;
BOOL G_versionSpecified = FALSE;



//-----------------------------------------------------------------------------
// 4127 -- Conditional Expression is Constant warning
//-----------------------------------------------------------------------------
#define WHILE(constant) \
__pragma(warning(disable: 4127)) while(constant); __pragma(warning(default: 4127))


#define USAGE  \
"Usage: nonpnpapp <-V version> <-l> \n" \
       " -V version  {if no version is specified the version specified in the build environment will be used.}\n" \
       "    The version is the version of the KMDF coinstaller to use \n"  \
       "    The format of version  is MMmmm where MM -- major #, mmm - serial# \n" \
       " -l  { option to continuously read & write to the file} \n"

BOOL
ValidateCoinstallerVersion(
    _In_ PSTR Version
    )
{   BOOL ok = FALSE;
    INT i;

    for(i= 0; i<MAX_VERSION_SIZE ;i++){
        if( ! IsCharAlphaNumericA(Version[i])) {
            break;
        }
    }
    if (i == (MAX_VERSION_SIZE -sizeof(CHAR))) {
        ok = TRUE;
    }
    return ok;
}

PCHAR
GetCoinstallerVersion(
    VOID
    )
{
    if (FAILED( StringCchPrintf(G_coInstallerVersion,
                                MAX_VERSION_SIZE,
                                "%02d%03d",    // for example, "01009"
                                KMDF_VERSION_MAJOR,
                                KMDF_VERSION_MINOR)))
    {
        printf("StringCchCopy failed with error \n");
    }

    return (PCHAR)&G_coInstallerVersion;
}

HANDLE devHandle = NULL;
CHAR driverLocation [MAX_PATH];
HMODULE library = NULL;

VOID
OpenDev()
{
    DWORD    errNum = 0;
    BOOL     ok;
    PCHAR    coinstallerVersion;

    if (!G_versionSpecified ) {
        coinstallerVersion = GetCoinstallerVersion();

        //
        // if no version is specified or an invalid one is specified use default version
        //
        printf("No version specified. Using default version:%s\n",
               coinstallerVersion);

    } else {
        coinstallerVersion = (PCHAR)&G_coInstallerVersion;
    }

    //
    // open the device
    //
    devHandle = CreateFile(DEVICE_NAME,
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         NULL,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);

    if(devHandle == INVALID_HANDLE_VALUE) {

        errNum = GetLastError();

        if (!(errNum == ERROR_FILE_NOT_FOUND ||
                errNum == ERROR_PATH_NOT_FOUND)) {

            printf("CreateFile failed!  ERROR_FILE_NOT_FOUND = %d\n",
                   errNum);
            return ;
        }

        //
        // Load WdfCoInstaller.dll.
        //
        library = LoadWdfCoInstaller();

        if (library == NULL) {
            printf("The WdfCoInstaller%s.dll library needs to be "
                   "in same directory as nonpnpapp.exe\n", coinstallerVersion);
            return;
        }

        //
        // The driver is not started yet so let us the install the driver.
        // First setup full path to driver name.
        //
        ok = SetupDriverName( driverLocation, MAX_PATH );

        if (!ok) {
            return ;
        }

        ok = ManageDriver( DRIVER_NAME,
                           driverLocation,
                           DRIVER_FUNC_INSTALL );

        if (!ok) {

            printf("Unable to install driver. \n");

            //
            // Error - remove driver.
            //
            ManageDriver( DRIVER_NAME,
                          driverLocation,
                          DRIVER_FUNC_REMOVE );
            return;
        }

        devHandle = CreateFile( DEVICE_NAME,
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL );

        if (devHandle == INVALID_HANDLE_VALUE) {
            printf ( "Error: CreatFile Failed : %d\n", GetLastError());
            return;
        }
    }
}

VOID
CloseDev()
{
    CloseHandle ( devHandle );

    ManageDriver( DRIVER_NAME,
                  driverLocation,
                  DRIVER_FUNC_REMOVE );

    if ( library ) {
        UnloadWdfCoInstaller( library );
    }
}

extern void ExecuteNop();

void uFunc()
{
	long long i;
	for (i = 0; i < 1000000000; i++)
		ExecuteNop();
}


VOID __cdecl
main()
{
	OpenDev();

	//ReadIDT();
	//ReadFunc();

	//CloseDev();

//#if 0

    InitDev();

	unsigned long long fsBaseL = ReadFSBase();
	unsigned long long gsBaseL = ReadGSBase();

	WriteFSBase(0x102345);
	//WriteGSBase(102345);

	//EnterU();
	uFunc();
	//Sleep(1);

	unsigned long long gsBaseU = ReadGSBase();
	unsigned long long fsBaseU = ReadFSBase();

	WriteFSBase(fsBaseL);
	WriteGSBase(gsBaseL);

	//EnterL(gsBaseL);
	

	printf("FL:%llx GL:%llx FU:%llx GU:%llx\n", fsBaseL, gsBaseL, fsBaseU, gsBaseU);


	CloseDev();

    return;
//#endif
}


enum { SystemModuleInformation = 11 };

typedef struct _RTL_PROCESS_MODULE_INFORMATION {
    ULONG Section;
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    CHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

typedef int (*NtQuerySystemInformationFunc)(
    _In_      DWORD SystemInformationClass,
    _Inout_   PVOID                    SystemInformation,
    _In_      ULONG                    SystemInformationLength,
    _Out_opt_ PULONG                   ReturnLength
);


VOID
EnterL(ULONG64 gBase)
{
    BOOL bRc;
    ULONG bytesReturned;

	WriteGSBase(gBase);
    bRc = DeviceIoControl ( devHandle,
                            (DWORD) IOCTL_NONPNP_SET_LIBRARY_GS,
                            NULL,
                            0,
                            NULL,
                            0,
                            &bytesReturned,
                            NULL
                            );

    if ( !bRc )
    {
        printf ( "Error in DeviceIoControl2 : %d\n", GetLastError());
    }
}

VOID
EnterU()
{
    BOOL bRc;
    ULONG bytesReturned;

    bRc = DeviceIoControl ( devHandle,
                            (DWORD) IOCTL_NONPNP_SET_PUBLIC_GS,
                            NULL,
                            0,
                            NULL,
                            0,
                            &bytesReturned,
                            NULL
                            );

    if ( !bRc )
    {
        printf ( "Error in DeviceIoControl3 : %d\n", GetLastError());
    }
}

VOID
ReadIDT()
{
    unsigned long long OutputBuffer[256];
    char InputBuffer[200];
    BOOL bRc;
    ULONG bytesReturned;

	int i;

	if (!devHandle)
	{
		printf("dev not opened!");
		return;
	}

    NtQuerySystemInformationFunc NtQuerySystemInformation = NULL;
    HMODULE hNtdll = NULL;
    ULONG64 KernelBase = 0;
    RTL_PROCESS_MODULES ModuleInfo = { 0 };

    // Get the address of NtQuerySystemInformation
    hNtdll = GetModuleHandle("ntdll");
    NtQuerySystemInformation = (NtQuerySystemInformationFunc)GetProcAddress(hNtdll, "NtQuerySystemInformation");

    // Get the base address of the kernel
    NtQuerySystemInformation(SystemModuleInformation, &ModuleInfo, sizeof(ModuleInfo), NULL);
    KernelBase = (ULONG64)ModuleInfo.Modules[0].ImageBase;

	printf("KernelBase:%llx\n", KernelBase);
    printf("\nCalling DeviceIoControl PATCH_KERNEL\n");

    memset(OutputBuffer, 0, sizeof(OutputBuffer));
	((unsigned long long*)InputBuffer)[0] = KernelBase;
	((unsigned long long*)InputBuffer)[1] = 0xfffaa77770;
	((unsigned long long*)InputBuffer)[2] = 0xfffee77770;


    bRc = DeviceIoControl ( devHandle,
                            (DWORD) IOCTL_NONPNP_READ_IDT,
                            InputBuffer,
                            24,
                            OutputBuffer,
                            sizeof( OutputBuffer),
                            &bytesReturned,
                            NULL
                            );

    if ( !bRc )
    {
        printf ( "Error in DeviceIoControl1 : : %d", GetLastError());
    }
	for (i = 0; i < 256; i++)
	{
		printf("%d # %llx\n", i, OutputBuffer[i] - KernelBase);
	}
}



VOID
ReadFunc()
{
    unsigned long long OutputBuffer[256];
    char InputBuffer[200];
    BOOL bRc;
    ULONG bytesReturned;

	int i;

	if (!devHandle)
	{
		printf("dev not opened!");
		return;
	}

    NtQuerySystemInformationFunc NtQuerySystemInformation = NULL;
    HMODULE hNtdll = NULL;
    ULONG64 KernelBase = 0;
    RTL_PROCESS_MODULES ModuleInfo = { 0 };

    // Get the address of NtQuerySystemInformation
    hNtdll = GetModuleHandle("ntdll");
    NtQuerySystemInformation = (NtQuerySystemInformationFunc)GetProcAddress(hNtdll, "NtQuerySystemInformation");

    // Get the base address of the kernel
    NtQuerySystemInformation(SystemModuleInformation, &ModuleInfo, sizeof(ModuleInfo), NULL);
    KernelBase = (ULONG64)ModuleInfo.Modules[0].ImageBase;

	printf("KernelBase:%llx\n", KernelBase);
    printf("\nCalling DeviceIoControl PATCH_KERNEL\n");

    memset(OutputBuffer, 0, sizeof(OutputBuffer));
	((unsigned long long*)InputBuffer)[0] = KernelBase;
	((unsigned long long*)InputBuffer)[1] = 0xfffaa77770;
	((unsigned long long*)InputBuffer)[2] = 0xfffee77770;


    bRc = DeviceIoControl ( devHandle,
                            (DWORD) IOCTL_NONPNP_READ_FUNC,
                            InputBuffer,
                            24,
                            OutputBuffer,
                            sizeof( OutputBuffer),
                            &bytesReturned,
                            NULL
                            );

    if ( !bRc )
    {
        printf ( "Error in DeviceIoControl1 : : %d", GetLastError());
    }
	for (i = 0; i < 64; i++)
	{
		printf("%hhx ", ((unsigned char*)OutputBuffer)[i]);
	}
}


VOID
InitDev()
{
    char OutputBuffer[100];
    char InputBuffer[200];
    BOOL bRc;
    ULONG bytesReturned;

	if (!devHandle)
	{
		printf("dev not opened!");
		return;
	}

    NtQuerySystemInformationFunc NtQuerySystemInformation = NULL;
    HMODULE hNtdll = NULL;
    ULONG64 KernelBase = 0;
    RTL_PROCESS_MODULES ModuleInfo = { 0 };

    // Get the address of NtQuerySystemInformation
    hNtdll = GetModuleHandle("ntdll");
    NtQuerySystemInformation = (NtQuerySystemInformationFunc)GetProcAddress(hNtdll, "NtQuerySystemInformation");

    // Get the base address of the kernel
    NtQuerySystemInformation(SystemModuleInformation, &ModuleInfo, sizeof(ModuleInfo), NULL);
    KernelBase = (ULONG64)ModuleInfo.Modules[0].ImageBase;

	printf("KernelBase:%llx\n", KernelBase);
    printf("\nCalling DeviceIoControl PATCH_KERNEL\n");

    memset(OutputBuffer, 0, sizeof(OutputBuffer));
	((unsigned long long*)InputBuffer)[0] = KernelBase;
	((unsigned long long*)InputBuffer)[1] = 0xfffaa77770;
	((unsigned long long*)InputBuffer)[2] = 0xfffee77770;


    bRc = DeviceIoControl ( devHandle,
                            (DWORD) IOCTL_NONPNP_METHOD_PATCH_KERNEL,
                            InputBuffer,
                            24,
                            OutputBuffer,
                            sizeof( OutputBuffer),
                            &bytesReturned,
                            NULL
                            );

    if ( !bRc )
    {
        printf ( "Error in DeviceIoControl1 : : %d", GetLastError());
    }
}
/*
BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{
	UNREFERENCED_PARAMETER(hinstDLL);
	UNREFERENCED_PARAMETER(lpReserved);
	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
*/
