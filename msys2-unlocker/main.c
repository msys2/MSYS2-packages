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

#define WIN32_MEAN_AND_LEAN

#include <windows.h>
#include <psapi.h>
#include <shellapi.h>

#include <stdio.h>
#include <stdint.h>

#include "resource.h"

typedef enum {
  kUnlockHelpOnly,
  kUnlockNone,
  kUnlockGUI,
  kUnlockForce,
} kUnlockAction;

typedef enum {

  /* All is good */
  kSuccess = 0,

  /* User errors */
  kUserNoFiles,
  kUserTooManyFiles,
  kUserExited,
  kUserCancelled,

  /* Failures */
  kFailEnumProc,
  kFailForce,

} kExitCode;

typedef struct {
  DWORD id;
  char* name;
  uint32_t lockedFilesMask;
} ProcessDetails;

/* Global variables plus things
   that are updated dynamically */
typedef struct {
  HWND appHWnd;

  /* EnumerateProcesses() */
  size_t num_processes;
  ProcessDetails* p_processes;

} GlobalState;

/* Values from the commandline */
typedef struct {
  kUnlockAction unlock_action;
  size_t num_filenames;
  char** p_filenames;
} Arguments;

enum {
  kProcessIncrement = 512,
  kModulesIncrement = 512,
} kTweaks;

HWND FindConsoleHandle(void);

void PrintHelp() {
  printf(
      "msys2-unlocker: Informs of and / or terminates\n"
      "                processes locking modules that\n"
      "                are passed in from the cmdline\n"
      "\n"
      "Usage: msys2-unlocker [options] files...\n"
      "Options:\n"
      "  --help            Display this message\n"
      "  --force           Force quit locking processes\n"
      "  --gui             Present unlocking GUI\n");
}

kExitCode ProcessArguments(int argc, char* argv[], Arguments* args) {
  typedef enum {
    kArgParseFirst,
    kArgParseSwitches = kArgParseFirst,
    kArgParseFilenames,
    kArgParseCount,
  } kArgParsePass;

  args->unlock_action = kUnlockNone;
  args->num_filenames = 0;
  args->p_filenames = NULL;
  int argi;
  kArgParsePass pass;
  size_t num_filename_char = 0;
  char* filename_chars;

  for (pass = kArgParseFirst; pass < kArgParseCount; ++pass) {
    if (pass == kArgParseFilenames) {
      args->p_filenames = (char**)malloc((sizeof(char*) * args->num_filenames) +
                                         (sizeof(char) * num_filename_char));
      filename_chars = (char*)&args->p_filenames[args->num_filenames];
      args->num_filenames = 0;
    }
    for (argi = 1; argi < argc; ++argi) {
      if (argv[argi] != NULL) {
        if (!strcmp("--gui", argv[argi])) {
          args->unlock_action = kUnlockGUI;
        } else if (!strcmp("--force", argv[argi])) {
          args->unlock_action = kUnlockForce;
        } else if (!strcmp("--help", argv[argi])) {
          PrintHelp();
          args->unlock_action = kUnlockHelpOnly;
          return (kSuccess);
        } else {
          if (pass == kArgParseSwitches) {
            ++args->num_filenames;
            if (args->num_filenames > 32) {
              return (kUserTooManyFiles);
            }
            num_filename_char += strlen(argv[argi]) + 1;
          } else if (pass == kArgParseFilenames) {
            args->p_filenames[args->num_filenames++] = filename_chars;
            strcpy(filename_chars, argv[argi]);
            filename_chars += strlen(argv[argi]) + 1;
          }
        }
      }
    }
  }
  if (args->num_filenames == 0) {
    return (kUserNoFiles);
  }
  return (kSuccess);
}

ssize_t EnumerateOpenProcessModules(HANDLE proc_handle, HMODULE** pp_modules) {
  ssize_t num_modules = 0;
  HMODULE* modules = NULL;
  size_t bytes_used = 0;
  BOOL result = FALSE;

  if (pp_modules == NULL) {
    return 0;
  }

  do {
    DWORD bytes_needed;
    num_modules += kModulesIncrement;
    bytes_used = num_modules * sizeof(DWORD);
    modules = (HMODULE*)alloca(bytes_used);
    if (modules == NULL) {
      CloseHandle(proc_handle);
      return -ENOMEM;
    }

    result =
        EnumProcessModules(proc_handle, modules, bytes_used, &bytes_needed);

    if (result != FALSE) {
      if (bytes_used == bytes_needed) {
        /* In this case, can't assume that all the processes were enumerated. */
        result = FALSE;
      }
    }
    if (result != FALSE) {
      num_modules = bytes_needed / sizeof(DWORD);
      bytes_used = bytes_needed;
    }
  } while (result == FALSE);

  *pp_modules = malloc(bytes_used);
  if (*pp_modules == NULL) {
    CloseHandle(proc_handle);
    return -ENOMEM;
  }
  memcpy(*pp_modules, modules, bytes_used);

  return num_modules;
}

/* Returns the count, -GetLastError () or -ENOMEM */
ssize_t EnumerateProcessModules(DWORD processId, HMODULE** pp_modules) {
  if (pp_modules == NULL) {
    return 0;
  }

  HANDLE proc_handle = OpenProcess(
      PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processId);
  if (proc_handle == NULL) {
    return -(ssize_t)GetLastError();
  }

  ssize_t num_modules = EnumerateOpenProcessModules(proc_handle, pp_modules);

  CloseHandle(proc_handle);

  return num_modules;
}

uint32_t GetProcessConflictingModules(DWORD processId, char** filenames,
                                      size_t num_files) {
  HMODULE* modules;
  uint32_t result = 0;
  ssize_t num_modules = EnumerateProcessModules(processId, &modules);
  if (num_modules <= 0) {
    return 0;
  }

  HANDLE proc_handle = OpenProcess(
      PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processId);
  if (proc_handle == NULL) {
    return -(ssize_t)GetLastError();
  }

  size_t fid;
  ssize_t mid;
  char module_name[1024];
  for (mid = 0; mid < num_modules; ++mid) {
    if (GetModuleFileNameExA(proc_handle, modules[mid], module_name,
                             sizeof(module_name) / sizeof(module_name[0])))
      printf("\t%s\n", module_name);
    for (fid = 0; fid < num_files; ++fid) {
      if (!strcmp(module_name, filenames[fid])) {
        result |= (1 << fid);
      }
    }
  }

  CloseHandle(proc_handle);

  free(modules);
  return (result);
}

uint32_t GetOpenProcessConflictingModules(Arguments* args, HANDLE proc_handle) {
  HMODULE* modules;
  uint32_t result = 0;
  ssize_t num_modules = EnumerateOpenProcessModules(proc_handle, &modules);

  if (modules) {
    size_t fid;
    ssize_t mid;
    char module_name[1024];
    for (mid = 0; mid < num_modules; ++mid) {
      if (GetModuleFileNameExA(proc_handle, modules[mid], module_name,
                               sizeof(module_name) / sizeof(module_name[0])))
        printf("\t%s\n", module_name);
      for (fid = 0; fid < args->num_filenames; ++fid) {
        if (!strcmp(module_name, args->p_filenames[fid])) {
          result |= (1 << fid);
        }
      }
    }
    free(modules);
  }

  return (result);
}

/* Returns error codes as negative numbers.
   -1 == alloca failed (out of stack space)
   -2 == malloc failed (out of heap space)
 */
int EnumerateProcesses(Arguments* args, GlobalState* state) {
  size_t i, num_processes = state->num_processes;
  DWORD* processes = NULL;
  size_t bytes_used = 0;
  BOOL result = FALSE;
  BOOL old_state_invalid = FALSE; /* Don't want to allocate un-necessarily. */

  do {
    DWORD bytes_needed;
    bytes_used = num_processes * sizeof(DWORD);
    processes = (DWORD*)alloca(bytes_used);
    if (processes == NULL) {
      return -2;
    }

    result = EnumProcesses(processes, bytes_used, &bytes_needed);
    if (result != FALSE) {
      if (bytes_used == bytes_needed) {
        /* In this case, can't assume that all the processes were enumerated. */
        result = FALSE;
      }
    }
    if (result != FALSE) {
      num_processes = bytes_needed / sizeof(DWORD);
      bytes_used = bytes_needed;
    }
    num_processes += kProcessIncrement;
  } while (result == FALSE);

  if (num_processes != state->num_processes) {
    old_state_invalid = TRUE;
  }

  size_t filename_length = 0;
  for (i = 0; i < num_processes; ++i) {
    HANDLE proc_handle =
        OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE,
                    processes[i]);
    char module_path[1024];
    module_path[0] = '\0';
    uint32_t lockedFiles = 0;
    if (proc_handle != NULL) {
      GetModuleFileNameExA(proc_handle, NULL, &module_path[0],
                           sizeof(module_path));
      lockedFiles = GetOpenProcessConflictingModules(args, proc_handle);
    }
    CloseHandle(proc_handle);

    filename_length += strlen(module_path) + 1;
    if (old_state_invalid == FALSE) {
      if ((state->p_processes != NULL) &&
          (processes[i] != state->p_processes[i].id ||
           strcmp(module_path, state->p_processes[i].name))) {
        old_state_invalid = TRUE;
      }
    }
  }

  state->p_processes = (DWORD*)realloc(state->p_processes, bytes_used);
  if (state->p_processes == NULL) {
    return -3;
  }
  memcpy(state->p_processes, processes, bytes_used);
  return (num_processes);
}

/* NYI: http://processhacker.sourceforge.net/forums/viewtopic.php?f=8&t=1584
        http://msdn.microsoft.com/en-us/library/windows/desktop/aa813708.aspx
ssize_t GetHandlesForProcess(DWORD processId) {
    HANDLE proc_handle = OpenProcess (PROCESS_QUERY_INFORMATION |
PROCESS_VM_READ, FALSE, processId);
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

DWORD_PTR GetProcessIcon() {
  /*
    DWORD_PTR icon = SHGetFileInfo(pszPath,
        DWORD dwFileAttributes,
        _Inout_  SHFILEINFO *psfi,
        UINT cbFileInfo,
        UINT uFlags
      );
  */
  return 0;
}

HWND FindConsoleHandle() {
  const char alphabet[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  char title[33];
  char old_title[512];

  size_t size = sizeof(title);
  size_t i;
  for (i = 0; i < size - 1; ++i) {
    title[i] = alphabet[rand() % (int)(sizeof(alphabet) - 1)];
  }
  title[i] = '\0';

  if (GetConsoleTitleA(old_title, sizeof(old_title) / sizeof(old_title[0])) ==
      0) {
    return NULL;
  }
  SetConsoleTitleA(title);
  Sleep(40);
  HWND wnd = FindWindowA(NULL, title);
  SetConsoleTitleA(old_title);
  return wnd;
}

INT_PTR CALLBACK
DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  GlobalState* state = (GlobalState*)lParam;
  (void)state;
  switch (uMsg) {
    case WM_INITDIALOG: {
      return TRUE;
      break;
    }

    case WM_COMMAND: {
      switch (wParam) {
        case IDOK: {
          EndDialog(hwndDlg, 0);
          return TRUE;
          break;
        }
      }
      break;
    }

    case WM_SIZE: {
      HWND hEdit;
      RECT rcClient;

      GetClientRect(hwndDlg, &rcClient);

      hEdit = GetDlgItem(hwndDlg, IDC_UNLOCKERDIALOG);
      SetWindowPos(hEdit, NULL, 0, 0, rcClient.right, rcClient.bottom,
                   SWP_NOZORDER);
      break;
    }

    case WM_CLOSE: {
      EndDialog(hwndDlg, kUserExited);
      return TRUE;
      break;
    }

    default: { break; }
  }

  return FALSE;
}

kExitCode UpdateState(Arguments* args, GlobalState* state) {
  ssize_t i, num_processes = EnumerateProcesses(args, state);
  if (num_processes < 0) {
    return (kFailEnumProc);
  }
  for (i = 0; i < num_processes; ++i) {
    uint32_t conflicts = GetProcessConflictingModules(
        state->p_processes[i].id, args->p_filenames, args->num_filenames);
    if (conflicts) {
      if (args->unlock_action == kUnlockGUI) {
        INT_PTR result =
            DialogBoxParamA(NULL, MAKEINTRESOURCEA(IDC_UNLOCKERDIALOG),
                            state->appHWnd, DialogProc, (LPARAM)state);
        if (result == kUserExited) {
          return (kUserExited);
        }
      }
    }
  }
  return (kSuccess);
}

int main(int argc, char* argv[]) {
  Arguments args;
  GlobalState state;
  state.num_processes = 0;
  state.p_processes = NULL;

  state.appHWnd = FindConsoleHandle();

  kExitCode args_result = ProcessArguments(argc, argv, &args);
  if (args_result == kUserNoFiles) {
    fprintf(stderr, "No filenames specified.\n");
    return (kUserNoFiles);
  } else if (args_result == kUserTooManyFiles) {
    fprintf(stderr, "Too many filenames specified, max is 32.\n");
    return (kUserTooManyFiles);
  }

  if (args.unlock_action == kUnlockHelpOnly) {
    return (kSuccess);
  }

  do {
    kExitCode update_result = UpdateState(&args, &state);
    if (update_result == kFailEnumProc) {
      fprintf(stderr, "Failed to enumerate processes.\n");
      return (kFailEnumProc);
    }

  } while (0);

  return (kSuccess);
}
