/*
 This program creates an empty file using the
 NtCreateFile function of Windows NT's Native API.

 Note that this function is documented in the
 Windows DDK.

 Original C++ version at
 http://analysis.seclab.tuwien.ac.at/Resources/NativeApiTest.cpp
 Ported to C, cygwin/miniddk.h, and wine by Dan Kegel

 Halfway to a minimal conformance test for NtCreateFile.
 Prints out one of three codes for each check:
 FAIL if something's wrong
 PASS if something went right
 SKIP if some test had to be skipped because the system didn't support it

 To compile with cygwin:
   gcc NativeApiTest.c -I.
 To compile with winegcc, copy into top of wine tree, then do:
   winegcc NativeApiTest.c -Iinclude -Llibs/wine
 To compile with Visual C++ Express:
   can't do it from commandline, since Visual C++ Express ignores /GS- flag,
   do: New Project / Win32 Console, named "demo",
   delete default files from left pane,
   Project / Add Existing Object (this file),
   Project / demo Properties / Precompiled Headers / Not Using Precompiled Headers,
   Project / Build demo

 Then just run the executable.  The first time you run it, it should pass,
 but the 2nd time it should fail unless you delete c:\testfile.txt first.
*/
#define UNICODE
#include <iostream>
#include <conio.h>

#include "windows.h"
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)  /* FIXME: wine lacks ntdef.h */
#include "winternl.h"
#include <stdio.h>
#include "status-map.h"

/* Standard headers above declare NtCreateFile, but we want a
 * function pointer, so we gotta declare it ourselves here.
 */
typedef NTSTATUS (__stdcall *NtCreateFilePtr)(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength );

typedef VOID (__stdcall *RtlInitUnicodeStringPtr) (
    IN OUT PUNICODE_STRING  DestinationString,
    IN PCWSTR  SourceString );

#define BUFSIZE 4096

//int main(int argc, char *argv[])
int main(int argc, char *argv[])
{
   LPWSTR *szArglist;
   int nArgs;
   int i;


   std::wstring fileName = L"";

   szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
   if( NULL == szArglist)
   {
      wprintf(L"CommandLineToArgvW failed\n");
      return 0;
   }
   else {
     if (nArgs < 2) {
        std::wcout << L"Expected a filename. Did not receive enough arguments.\n";
        return(1);
     }

     //now resolve to a complete path

     WCHAR buffer[BUFSIZE] = L"";
     GetFullPathNameW(szArglist[1], BUFSIZE, buffer, NULL);

     fileName = L"\\??\\" + std::wstring(buffer);
     //for( i=0; i<nArgs; i++) {
     //  std::wcout << i << L": " << szArglist[i] << L"\n";
     //}
   }

   // Free memory allocated for CommandLineToArgvW arguments.

   //std::wstring longString1 = L"\\??\\" + std::wstring(GetCommandLine());

   LocalFree(szArglist);

   //return(1);

    //

    std::wcout << L"Checking file: '" << fileName << L"'\n";


    UNICODE_STRING fn;
    OBJECT_ATTRIBUTES object;
    IO_STATUS_BLOCK ioStatus;
    NtCreateFilePtr pNtCreateFile;
    RtlInitUnicodeStringPtr pRtlInitUnicodeString;
    HANDLE out;
    NTSTATUS status;
    HMODULE hMod;

    /* Get access to ntdll functions.  This will fail on Win9x. */
    hMod = LoadLibraryA("ntdll.dll");
    if (!hMod) {
	printf("SKIP: Could not load ntdll.dll\n");
	exit(0);
    }
    pNtCreateFile = (NtCreateFilePtr) GetProcAddress(hMod, "NtCreateFile");
    if (!pNtCreateFile) {
	printf("FAIL: Could not locate NtCreateFile\n");
	exit(1);
    }
    pRtlInitUnicodeString = (RtlInitUnicodeStringPtr) GetProcAddress(hMod,
						"RtlInitUnicodeString");

    /* Create a file using NtCreateFile */
    memset(&ioStatus, 0, sizeof(ioStatus));
    memset(&object, 0, sizeof(object));
    object.Length = sizeof(object);
    object.Attributes = OBJ_CASE_INSENSITIVE;
    pRtlInitUnicodeString(&fn, fileName.c_str());

    //pRtlInitUnicodeString(&fn, L"\\??\\C:\\Users\\bruce.eng\\Documents\\mvpc-kpi\\kpi-data\\logs\\get-altacs-data.log.2022-05-10");
    object.ObjectName = &fn;

    //status = pNtCreateFile(&out, GENERIC_WRITE, &object, &ioStatus, NULL,
	//	      FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE, 0, NULL,
	//	      0);

//FILE_READ_ATTRIBUTES
	status = pNtCreateFile(&out, FILE_READ_DATA, &object, &ioStatus, NULL,
	  FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, FILE_OPEN, 0, NULL,
	  0);



    if (!NT_SUCCESS(status)) {
	  printf("FAIL: Could not open file.\n");

	  const char *statusString = status_to_string(status);

	  std::cout << "Status: " << (void *)status;
	  if (statusString) {
        std::cout << " (" << statusString << ")\n";
	  }
	  else {
        std::cout << " (STATUS_UNKNOWN)\n";
	  }

	  //std::cout << "Press a key: ";
	  //getch();

	  //0xc0000056 = STATUS_DELETE_PENDING (Normally this shows up as access denied)
	  //0xc0000034 = STATUS_OBJECT_NAME_NOT_FOUND (Doesn't exist)
	  //0xc0000035 = STATUS_OBJECT_NAME_COLLISION (If you try FILE_CREATE and the file already exists)
	  //0xc0000043 = STATUS_SHARING_VIOLATION (Another process has the file opened and didn't set the FILE_SHARE_READ flag)
	  //0xc0000022 = STATUS_ACCESS_DENIED (If you don't have permission to open it)

	  exit(1);
    }
    printf("PASS: Successfully opened the file!\n");
    //std::cout << "Press a key: ";
	//getch();
    return 0;
}
