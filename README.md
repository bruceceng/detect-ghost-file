# detect-ghost-file
Detect Windows NTFS files that have STATUS_DELETE_PENDING as well as other detailed status codes

## Motivation
I was running into log files that I could not open. They appeared to be valid windows files, but any attempt to open them ended in access denied errors.

After much research, I discovered that it is possible for "ghost files" to be created on a NTFS file system. If a file is deleted while its handle is still opened by something like the Windows search indexer, it might become a "ghost file" which won't actually disappear until the next reboot.

In this state, opening it with NtCreateFile / ZwCreateFile (the Windows Kernel function for opening files) will return STATUS_DELETE_PENDING. However most other Windows API functions like GetLastError (I think...) will just map this to "access denied". Therefore, this utility will open a file and give a more informative status string directly from the result of NtCreateFile.

## Build
This was built on Windows 10 using codeblocks-20.03mingw-32bit-nosetup which can be readily downloaded. The .cbp project file should be openable in codeblocks.

## Run
If you don't feel like building from scratch, you can try using the contained executables directly. Just run the executable and pass an argument of the file you want to attempt to open:

> detect-ghost-file.exe "c:\User\bruce\documents\my-file.txt"

## Results
Will print the resolved name of the file its attempting to open. The \??\ means womething in windows (its not a mistake) and you can google it if you are curious...

Then will attempt to open the file in a read only mode. If it fails, will return a status code as well as a readable status string.

Here are some possible return values:

0xc0000056 = STATUS_DELETE_PENDING (Normally this shows up as access denied)

0xc0000034 = STATUS_OBJECT_NAME_NOT_FOUND (Doesn't exist)

0xc0000035 = STATUS_OBJECT_NAME_COLLISION (If you try FILE_CREATE and the file already exists)

0xc0000043 = STATUS_SHARING_VIOLATION (Another process has the file opened and didn't set the FILE_SHARE_READ flag)

0xc0000022 = STATUS_ACCESS_DENIED (If you don't have permission to open it)