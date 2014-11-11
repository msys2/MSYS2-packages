/*
   Looks for locks to specific files
   Only handles executables and dlls
   TODO :: Support for any file type
   .Written by Ray Donnelly in 2014.
   .Licensed under CC0 (and anything.
  .else you need to license it under).
      .No warranties whatsoever.
  .email: <mingw.android@gmail.com>.
 */

#include <stdio.h>
#include <tchar.h>
//#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <psapi.h>
#include <stdint.h>

/* Returns the count, -GetLastError () or -ENOMEM */
ssize_t EnumerateProcessModules(DWORD processId, HMODULE** pp_modules) {
    if (pp_modules == NULL) {
        return 0;
    }

    HANDLE proc_handle = OpenProcess (PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (proc_handle == NULL) {
        return -(ssize_t)GetLastError ();
    }

    ssize_t num_entries = 0;
    HMODULE* modules = NULL;
    size_t bytes_used = 0;
    BOOL result = FALSE;

    do {
        DWORD bytes_needed;
        num_entries += 1024;
        bytes_used = num_entries * sizeof(DWORD);
        modules = (HMODULE*)alloca (bytes_used);
        if (modules == NULL) {
            CloseHandle (proc_handle);
            return -2;
        }

        result = EnumProcessModules (proc_handle, modules, bytes_used, &bytes_needed);
        if (result != FALSE) {
            num_entries = bytes_needed / sizeof(DWORD);
            bytes_used = bytes_needed;
        }
    } while (result == FALSE);

    *pp_modules = malloc (bytes_used);
    if (*pp_modules == NULL) {
        CloseHandle (proc_handle);
        return -ENOMEM;
    }
    memcpy (*pp_modules, modules, bytes_used);
    CloseHandle (proc_handle);

    return num_entries;
}

uint32_t CheckForConflictingFiles(DWORD processId, char** filenames, size_t num_files) {
    HMODULE* modules;
    uint32_t result = 0;
    ssize_t num_modules = EnumerateProcessModules (processId, &modules);
    if (num_modules <= 0) {
        return 0;
    }

    HANDLE proc_handle = OpenProcess (PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (proc_handle == NULL) {
        return -(ssize_t)GetLastError ();
    }

    size_t fid;
    ssize_t mid;
    char module_name[1024];
    for (mid = 0; mid < num_modules; ++mid) {
        if (GetModuleFileNameExA (proc_handle, modules[mid], module_name,
                                 sizeof(module_name) / sizeof(module_name[0])))
        printf ("\t%s\n", module_name);
        for (fid = 0; fid < num_files; ++fid) {
            if (!strcmp(module_name, filenames[fid])) {
                result |= (1<<fid);
            }
        }
    }

    CloseHandle (proc_handle);

    free (modules);
    return result;
}

/* Returns error codes as negative numbers.
   -1 == invalid input, pp_processes is NULL
   -2 == alloca failed (out of stack space)
   -3 == malloc failed (out of heap space)
 */
ssize_t EnumerateProcesses(DWORD** pp_processes) {
    ssize_t num_entries = 0;
    DWORD* processes = NULL;
    size_t bytes_used = 0;
    BOOL result = FALSE;

    if (pp_processes == NULL) {
        return -1;
    }

    do {
        DWORD bytes_needed;
        num_entries += 1024;
        bytes_used = num_entries * sizeof(DWORD);
        processes = (DWORD*)alloca (bytes_used);
        if (processes == NULL) {
            return -2;
        }

        result = EnumProcesses (processes, bytes_used, &bytes_needed);
        if (result != FALSE) {
            num_entries = bytes_needed / sizeof(DWORD);
            bytes_used = bytes_needed;
        }
    } while (result == FALSE);

    *pp_processes = malloc (bytes_used);
    if (*pp_processes == NULL) {
        return -3;
    }
    memcpy (*pp_processes, processes, bytes_used);
    return num_entries;
}

/* NYI: http://processhacker.sourceforge.net/forums/viewtopic.php?f=8&t=1584
        http://msdn.microsoft.com/en-us/library/windows/desktop/aa813708.aspx
ssize_t GetHandlesForProcess(DWORD processId) {
    HANDLE proc_handle = OpenProcess (PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (proc_handle == NULL) {
        DWORD last_error = GetLastError ();
        return -last_error;
    }
    DWORD num_handles;
    BOOL result = GetProcessHandleCount (proc_handle, &num_handles);
    if (result == FALSE) {
        return -6;
    }
    return 0;
}
*/

typedef enum
{
    kArgParseFirst,
    kArgParseSwitches = kArgParseFirst,
    kArgParseFilenames,
    kArgParseCount,
} kArgParsePass;

typedef enum
{
    kUnlockNone,
    kUnlockGUI,
    kUnlockForce,
} kUnlockAction;

typedef struct
{
    uint32_t lockedFilesMask;
    DWORD    processId;
} LockingProcess;

HWND FindConsoleHandle()
 {
  static char temptitle[] = "{98C1C303-2A9E-11d4-9FF5-006067718D04}";
  char title[512];
  char me[64];
  sprintf(me, "%s-%08x", temptitle, GetCurrentProcessId());
  if(GetConsoleTitleA(title, sizeof(title)/sizeof(TCHAR)) == 0)
    return NULL;
  SetConsoleTitleA(me);
  HWND wnd = FindWindow(NULL, me);
  SetConsoleTitleA(title);
  return wnd;
 }


INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg)
        {
        case WM_INITDIALOG:
            return TRUE;
    
        case WM_COMMAND:
            switch(wParam)
            {
            case IDOK:
                EndDialog(hwndDlg, 0);
                return TRUE;
            }
            break;
        }
    
        return FALSE;
}

int main(int argc, char* argv[]) {
    int argi;
    kArgParsePass pass;
    kUnlockAction unlock_action = kUnlockNone;
    size_t num_filenames = 0;
    size_t num_filename_char = 0;
    char** p_filenames;
    char* filename_chars;

    /* Process arguments */
    for (pass = kArgParseFirst; pass < kArgParseCount; ++pass) {
        if (pass == kArgParseFilenames) {
            p_filenames = (char**)malloc ((sizeof(char*) * num_filenames) +
                                  (sizeof(char) * num_filename_char));
            filename_chars = (char*)&p_filenames[num_filenames];
            num_filenames = 0;
        }
        for (argi = 1; argi < argc; ++argi) {
            if (argv[argi] != NULL) {
                if (!strcmp ("--gui", argv[argi])) {
                    unlock_action = kUnlockGUI;
                } else if (!strcmp ("--force", argv[argi])) {
                    unlock_action = kUnlockForce;
                } else {
                    if (pass == kArgParseSwitches) {
                        ++num_filenames;
                        if (num_filenames > 32) {
                            fprintf(stderr, "Too many filenames, max is 32.\n");
                            return 1;
                        }
                        num_filename_char += strlen (argv[argi]) + 1;
                    }
                    else if (pass == kArgParseFilenames) {
                        p_filenames[num_filenames++] = filename_chars;
                        strcpy (filename_chars, argv[argi]);
                        filename_chars += strlen (argv[argi]) + 1;
                    }
                }
            }
        }
    }

    if (num_filenames == 0) {
        fprintf(stderr, "No filenames specified.\n");
        return 2;
    }

    do {
        DWORD* p_processes = NULL;
        ssize_t i, num_entries = EnumerateProcesses (&p_processes);
        if (num_entries < 0) {
            return -num_entries;
        }
        for (i = 0; i < num_entries; ++i) {
/*          ssize_t num_handles = GetHandlesForProcess (p_processes[i]); */
            uint32_t conflicts = CheckForConflictingFiles (p_processes[i], p_filenames, num_filenames);
            if (conflicts) {
                printf ("Yup, we conflict");
//                DialogBox(FindConsoleHandle(), "Hello There!", NULL, 0);
                int response = MessageBoxA( FindConsoleHandle(), 
                                           "File read error, continue?", 
                                           "File Error", 
                                           MB_ICONERROR | MB_YESNO);
                (void)response;
            }
        }
    } while (0);
    return 0;
}
