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

LONG
Parse(
    _In_ int argc,
    _In_reads_(argc) char *argv[]
    )
/*++
Routine Description:

    Called by main() to parse command line parms

Arguments:

    argc and argv that was passed to main()

Return Value:

    Sets global flags as per user function request

--*/
{
    int i;
    BOOL ok;
    LONG error = ERROR_SUCCESS;

    for (i=0; i<argc; i++) {
        if (argv[i][0] == '-' ||
            argv[i][0] == '/') {
            switch(argv[i][1]) {
            case 'V':
            case 'v':
                if (( (i+1 < argc ) &&
                      ( argv[i+1][0] != '-' && argv[i+1][0] != '/'))) {
                    //
                    // use version in commandline
                    //
                    i++;
                    ok = ValidateCoinstallerVersion(argv[i]);
                    if (!ok) {
                        printf("Not a valid format for coinstaller version\n"
                               "It should be characters between A-Z, a-z , 0-9\n"
                               "The version format is MMmmm where MM -- major #, mmm - serial#");
                        error = ERROR_INVALID_PARAMETER;
                        break;
                    }
                    if (FAILED( StringCchCopy(G_coInstallerVersion,
                                              MAX_VERSION_SIZE,
                                              argv[i]) )) {
                        break;
                    }
                    G_versionSpecified = TRUE;

                }
                else{
                    printf(USAGE);
                    error = ERROR_INVALID_PARAMETER;
                }
                break;
            case 'l':
            case 'L':
                G_fLoop = TRUE;
                break;
            default:
                printf(USAGE);
                error = ERROR_INVALID_PARAMETER;

            }
        }
    }
    return error;
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

VOID __cdecl
main(
    _In_ ULONG argc,
    _In_reads_(argc) PCHAR argv[]
    )
{
    HANDLE   hDevice;
    DWORD    errNum = 0;
    CHAR     driverLocation [MAX_PATH];
    BOOL     ok;
    HMODULE  library = NULL;
    LONG     error;
    PCHAR    coinstallerVersion;

    //
    // Parse command line args
    //   -l     -- loop option
    //
    if ( argc > 1 ) {// give usage if invoked with no parms
        error = Parse(argc, argv);
        if (error != ERROR_SUCCESS) {
            return;
        }
    }

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
    hDevice = CreateFile(DEVICE_NAME,
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         NULL,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);

    if(hDevice == INVALID_HANDLE_VALUE) {

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

        hDevice = CreateFile( DEVICE_NAME,
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL );

        if (hDevice == INVALID_HANDLE_VALUE) {
            printf ( "Error: CreatFile Failed : %d\n", GetLastError());
            return;
        }
    }

    DoIoctls(hDevice);

    //
    // Close the handle to the device before unloading the driver.
    //
    CloseHandle ( hDevice );

    //
    // Unload the driver.  Ignore any errors.
    //
    ManageDriver( DRIVER_NAME,
                  driverLocation,
                  DRIVER_FUNC_REMOVE );

    //
    // Unload WdfCoInstaller.dll
    //
    if ( library ) {
        UnloadWdfCoInstaller( library );
    }
    return;
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


extern ULONG64 ReadFSBase();
extern ULONG64 WriteFSBase(ULONG64);
extern ULONG64 ReadGSBase();
extern ULONG64 WriteGSBase(ULONG64);

VOID
EnterL(HANDLE hDevice, ULONG64 gBase)
{
    BOOL bRc;
    ULONG bytesReturned;

	WriteGSBase(gBase);
    bRc = DeviceIoControl ( hDevice,
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
EnterU(HANDLE hDevice)
{
    BOOL bRc;
    ULONG bytesReturned;

    bRc = DeviceIoControl ( hDevice,
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
DoIoctls(
    HANDLE hDevice
    )
{
    char OutputBuffer[100];
    char InputBuffer[200];
    BOOL bRc;
    ULONG bytesReturned;

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


    bRc = DeviceIoControl ( hDevice,
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
        return;
    }

	unsigned long long fsBaseL = ReadFSBase();
	unsigned long long gsBaseL = ReadGSBase();

	EnterU(hDevice);

	long long i;
	for (i = 0; i < 1000000000; i++);
	unsigned long long gsBaseU = ReadGSBase();
	unsigned long long fsBaseU = ReadFSBase();

	EnterL(hDevice, gsBaseL);

	printf("FL:%llx GL:%llx FU:%llx GU:%llx\n", fsBaseL, gsBaseL, fsBaseU, gsBaseU);

    return;

}
