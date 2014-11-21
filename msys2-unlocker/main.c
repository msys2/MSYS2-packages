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
#include <conio.h>
#include <stdint.h>

#include "resource.h"

typedef enum {

  /* Conflict detection. */
  kActionUnspeficied,
  /* --help */
  kActionHelpOnly,
  /* --query */
  kActionQuery,
  /* --observe, --ask, --force */
  kActionUnlocker,

} kAction;

typedef enum {

  /* Conflict detection. */
  kUnlockUnknown,
  /* --observe */
  kUnlockObserve,
  /* --ask */
  kUnlockAsk,
  /* --force */
  kUnlockForce,

} kUnlockMode;

typedef enum {

  /* --console */
  kInterfaceTUI,
  /* --gui */
  kInterfaceGUI,

} kInterface;

typedef enum {

  /* All is good */
  kSuccess = 0,
  /* Asked to move files, but the replacements
     do not exist. */
  kSuccessNoReplacements,

  /* User errors */
  kUserNoFiles,
  kUserTooManyFiles,
  kUserExited,
  kUserCancelled,
  kUserFilenameTooLong,
  kUserUnlockModeConflict,

  /* Failures */
  kFailInsufficientBuffer,
  kFailAlloca,
  kFailMalloc,
  kFailEnumProc,
  kFailForce,

} kExitCode;

typedef struct {
  DWORD id;
  char* name;
  uint32_t mask_locked_files;
} ProcessDetails;

typedef struct {
  float n_secs;
  BOOL displayed_splash;
  HRSRC splash_jpg;
  HBITMAP splash_jpg_bitmap;
} InterfaceState;

/* Global variables plus things
   that are updated dynamically */
typedef struct {
  HWND appHWnd;

  /* EnumerateLockingProcesses() */
  size_t num_locking_processes;
  ProcessDetails* p_locking_processes;

  /* Bitmask of files already moved. Locks  */
  uint32_t mask_moved_already;

} GlobalState;

/* Values from the commandline */
typedef struct {
  kAction action;

  /* The suffix of the new versions of the files
     to be moved into place. If NULL, then then
     moving doesn't happen. */
  char* move_suffix;

  kUnlockMode unlock_mode;
  kInterface interface_style;
  size_t num_filenames;
  char** p_filenames;

  /* Cached. */
  size_t strlen_longest_filename;
} Arguments;

enum {
  kProcessIncrement = 512,
  kModulesIncrement = 512,
} kTweaks;

HWND FindConsoleHandle(void);
BOOL LaunchedFromConsole(void);

#if defined(_DEBUG)
BOOL debug = TRUE;
#else
BOOL debug = FALSE;
#endif

#define dbg_printf(...)               \
  do {                                \
    if (debug == TRUE) {              \
      printf("%s(): ", __FUNCTION__); \
      printf(__VA_ARGS__);            \
      fflush(stdout);                 \
    }                                 \
  } while (0)

typedef BOOL (*OpenProcessFilterFn)(void* p_in_args, HANDLE proc_handle,
                                    HANDLE module_handle, void* p_out_value);

void PrintHelp() {
  printf(
      "msys2-unlocker: Informs of and / or terminates\n"
      "                processes locking file modules\n"
      "                passed in from the cmdline.\n"
      "                If --move-suffix was passed in\n"
      "                and those files exist, then it\n"
      "                replaces the passed in files\n"
      "                with the suffixed versions.\n"
      "\n"
      "Usage: msys2-unlocker [options] files...\n"
      "\n"
      "General options:\n"
      "  --help            Display this message.\n"
      "  --debug           Print debugging information.\n"
      "\n"
      "Operational options:\n"
      "  --query           List locking processes and exit.\n"
      "  --observe         Monitor locking processes.\n"
      "  --ask             Ask regarding termination.\n"
      "  --force           Force quit locking processes.\n"
      "  --move-suffix     The suffix of the new file version(s).\n"
      "\n"
      "Interface options:\n"
      "  --console         Use commandline only.\n"
      "  --gui             Use GUI.\n");
}

/* In case relative paths or forward slashes were passed. */
kExitCode FilenameToLongFilename(char* in, char* out, DWORD maxlen) {
  out[maxlen - 1] = '\0';
  _fullpath(out, in, maxlen);
  if (out[maxlen - 1] != '\0') {
    return (kUserFilenameTooLong);
  }
  return (kSuccess);
}

kExitCode ParseArguments(int argc, char* argv[], Arguments* args) {
  typedef enum {
    kArgParseFirst,
    kArgParseSwitches = kArgParseFirst,
    kArgParseFilenames,
    kArgParseCount,
  } kArgParsePass;

  args->unlock_mode = kUnlockUnknown;
  args->num_filenames = 0;
  args->p_filenames = NULL;
  args->move_suffix = NULL;
  if (LaunchedFromConsole()) {
    args->interface_style = kInterfaceTUI;
  } else {
    args->interface_style = kInterfaceGUI;
  }
  int argi;
  size_t i;
  kArgParsePass pass;
  size_t num_filename_char = 0;
  char* filename_chars;
  char long_filename[1024];
  kExitCode filename_result;

  for (pass = kArgParseFirst; pass < kArgParseCount; ++pass) {
    if (pass == kArgParseFilenames) {
      args->p_filenames = (char**)malloc((sizeof(char*) * args->num_filenames) +
                                         (sizeof(char) * num_filename_char));
      if (args->p_filenames == NULL) {
        return (kFailMalloc);
      }
      filename_chars = (char*)&args->p_filenames[args->num_filenames];
      args->num_filenames = 0;
    }
    for (argi = 1; argi < argc; ++argi) {
      if (argv[argi] != NULL) {
        if (!strcmp("--gui", argv[argi])) {
          args->interface_style = kInterfaceGUI;
        } else if (!strcmp("--tui", argv[argi])) {
          args->interface_style = kInterfaceTUI;
        } else if (!strcmp("--observe", argv[argi])) {
          args->action = kActionUnlocker;
          args->unlock_mode = kUnlockObserve;
        } else if (!strcmp("--ask", argv[argi])) {
          args->action = kActionUnlocker;
          args->unlock_mode = kUnlockAsk;
        } else if (!strcmp("--force", argv[argi])) {
          args->action = kActionUnlocker;
          args->unlock_mode = kUnlockForce;
        } else if (!strcmp("--help", argv[argi])) {
          args->action = kActionHelpOnly;
          return (kSuccess);
        } else if (!strcmp("--query", argv[argi])) {
          args->action = kActionQuery;
        } else if (!strncmp("--move-suffix=", argv[argi],
                            strlen("--move-suffix="))) {
          args->move_suffix =
              (char*)malloc(strlen(&argv[argi][strlen("--move-suffix=")]) + 1);
          if (args->move_suffix == NULL) {
            return (kFailMalloc);
          }
          strcpy(args->move_suffix, &argv[argi][strlen("--move-suffix=")]);
        } else {
          if (pass == kArgParseSwitches) {
            ++args->num_filenames;
            if (args->num_filenames > 32) {
              return (kUserTooManyFiles);
            }
            filename_result = FilenameToLongFilename(argv[argi], long_filename,
                                                     sizeof(long_filename));
            if (filename_result != kSuccess) {
              return (filename_result);
            }
            num_filename_char += strlen(long_filename) + 1;
          } else if (pass == kArgParseFilenames) {
            FilenameToLongFilename(argv[argi], long_filename,
                                   sizeof(long_filename));
            args->p_filenames[args->num_filenames++] = filename_chars;
            strcpy(filename_chars, long_filename);
            filename_chars += strlen(argv[argi]) + 1;
          }
        }
      }
    }
  }
  if (args->num_filenames == 0) {
    return (kUserNoFiles);
  }

  args->strlen_longest_filename = 0;
  for (i = 0; i < args->num_filenames; ++i) {
    if (args->strlen_longest_filename < strlen(args->p_filenames[i])) {
      args->strlen_longest_filename = strlen(args->p_filenames[i]);
    }
  }

  return (kSuccess);
}

ssize_t EnumerateOpenProcessModules(Arguments* args, HANDLE proc_handle,
                                    HMODULE** pp_modules, void* p_out_value,
                                    OpenProcessFilterFn filter_fn) {
  ssize_t num_modules = 0;
  HMODULE* modules = NULL;
  size_t bytes_used = 0;
  BOOL result = FALSE;
  BOOL use_alloca = FALSE;
  HMODULE* local_p_modules = 0;

  if (pp_modules == NULL) {
    use_alloca = TRUE;
    pp_modules = &local_p_modules;
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

  /* Now apply the filter */
  if (filter_fn) {
    ssize_t mid;
    filter_fn(args, NULL, NULL, p_out_value);
    for (mid = 0; mid < num_modules; ++mid) {
      if (filter_fn(args, proc_handle, modules[mid], p_out_value) == TRUE) {
      }
    }
  }

  if (use_alloca == FALSE) {
    *pp_modules = malloc(bytes_used);
    if (*pp_modules == NULL) {
      CloseHandle(proc_handle);
      return -ENOMEM;
    }
    memcpy(*pp_modules, modules, bytes_used);
  }

  return num_modules;
}

/* Returns TRUE if conflicts with any (Arguments*)p_in_args->p_filenames, and
  *sets
   *(uint32_t*)p_outvalues to the bitmask of conflicting ids. */
BOOL FilterConflictingModule(void* p_in_args, HANDLE proc_handle,
                             HANDLE module_handle, void* p_out_value) {
  Arguments* args = (Arguments*)p_in_args;
  uint32_t* p_mask_locked = (uint32_t*)p_out_value;

  if (proc_handle == NULL && module_handle == NULL) {
    p_mask_locked = 0;
    return FALSE;
  }

  size_t fid;
  char module_name[1024];
  if (GetModuleFileNameExA(proc_handle, module_handle, module_name,
                           sizeof(module_name) / sizeof(module_name[0]))) {
    printf("\t%s\n", module_name);
    for (fid = 0; fid < args->num_filenames; ++fid) {
      if (!strcmp(module_name, args->p_filenames[fid])) {
        *p_mask_locked |= (1 << fid);
      }
    }
  }
  if (*p_mask_locked) {
    return TRUE;
  }
  return FALSE;
}

uint32_t GetOpenProcessConflictingModules(Arguments* args, HANDLE proc_handle) {
  uint32_t mask_locked = 0;
  EnumerateOpenProcessModules(args, proc_handle, NULL, &mask_locked,
                              FilterConflictingModule);
  return (mask_locked);
}

kExitCode ExistMask(Arguments* args, char* suffix, uint32_t* p_result) {
  size_t i;
  char* filename_buf = (char*)alloca(args->strlen_longest_filename +
                                     (suffix == NULL ? 0 : strlen(suffix)) + 1);
  if (filename_buf == NULL) {
    return (kFailAlloca);
  }
  *p_result = 0;
  for (i = 0; i < args->num_filenames; ++i) {
    strcpy(filename_buf, args->p_filenames[i]);
    if (suffix != NULL) {
      strcat(filename_buf, suffix);
    }
    DWORD attrib = GetFileAttributesA(filename_buf);
    if (attrib != INVALID_FILE_ATTRIBUTES &&
        !(attrib & FILE_ATTRIBUTE_DIRECTORY)) {
      *p_result |= (1 << i);
    }
  }
  return (kSuccess);
}

kExitCode ExistMaskForLockedFiles(Arguments* args, uint32_t* p_result) {
  return (ExistMask(args, NULL, p_result));
}

kExitCode ExistMaskForReplacements(Arguments* args, uint32_t* p_result) {
  if (args->move_suffix == NULL) {
    *p_result = 0;
    return (kSuccess);
  }
  return (ExistMask(args, args->move_suffix, p_result));
}

kExitCode EnumerateLockingProcesses(Arguments* args, GlobalState* state) {
  size_t i;
  size_t num_processes = 0;
  DWORD* processes = NULL;
  size_t bytes_used = 0;
  BOOL result = FALSE;
  BOOL old_state_invalid =
      FALSE; /* Don't want to call realloc/malloc un-necessarily. */
  char* locking_filenames_scratch = alloca(4096);
  char* locking_filenames = locking_filenames_scratch;
  DWORD* locking_processes = NULL; /* Aliases processes */
  size_t filename_length = 0;
  size_t locking_count = 0;

  if (locking_filenames_scratch == NULL) {
    return (kFailAlloca);
  }

  do {
    DWORD bytes_needed;
    bytes_used = num_processes * sizeof(DWORD);
    processes = (DWORD*)alloca(bytes_used);
    if (processes == NULL) {
      return (kFailAlloca);
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

  locking_processes = processes;

  for (i = 0; i < num_processes; ++i) {
    HANDLE proc_handle =
        OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE,
                    processes[i]);
    char module_path[1024];
    module_path[0] = '\0';
    uint32_t mask_locked = 0;
    if (proc_handle != NULL) {
      mask_locked = GetOpenProcessConflictingModules(args, proc_handle);
      if (mask_locked) {
        GetModuleFileNameExA(proc_handle, NULL, &module_path[0],
                             sizeof(module_path));
        size_t this_filename_length = strlen(module_path) + 1;
        filename_length += this_filename_length;
        if (locking_filenames - locking_filenames_scratch - filename_length <=
            0) {
          CloseHandle(proc_handle);
          return (kFailInsufficientBuffer);
        }
        strcpy(locking_filenames, module_path);
        locking_filenames += this_filename_length;
        if (old_state_invalid == FALSE) {
          if ((state->p_locking_processes == NULL) ||
              (processes[i] != state->p_locking_processes[locking_count].id ||
               strcmp(module_path,
                      state->p_locking_processes[locking_count].name))) {
            old_state_invalid = TRUE;
          }
        }
        locking_processes[locking_count] = processes[i];
        locking_count++;
      }
    }
    CloseHandle(proc_handle);
  }

  /* Something changed */
  if (old_state_invalid == TRUE) {
    dbg_printf("Old:\n");
    for (i = 0; i < state->num_locking_processes; ++i) {
      dbg_printf("PID[%04x]: %s\n",
                 (unsigned int)state->p_locking_processes[i].id,
                 state->p_locking_processes[i].name);
    }
    bytes_used = locking_count * sizeof(ProcessDetails) + filename_length;
    state->p_locking_processes =
        (ProcessDetails*)realloc(state->p_locking_processes, bytes_used);
    if (state->p_locking_processes == NULL) {
      return (kFailMalloc);
    }
    locking_filenames = (char*)&state->p_locking_processes[locking_count];
    for (i = 0; i < locking_count; ++i) {
      state->p_locking_processes[i].id = processes[i];
      state->p_locking_processes[i].name = locking_filenames;
      strcpy(locking_filenames, locking_filenames_scratch);
      locking_filenames_scratch += strlen(locking_filenames_scratch) + 1;
      locking_filenames += strlen(locking_filenames) + 1;
    }
    state->num_locking_processes = locking_count;
    dbg_printf("New:\n");
    for (i = 0; i < state->num_locking_processes; ++i) {
      dbg_printf("PID[%04x]: %s\n",
                 (unsigned int)state->p_locking_processes[i].id,
                 state->p_locking_processes[i].name);
    }
  }
  return (kSuccess);
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
  if (wnd == NULL) {
    wnd = GetConsoleWindow();
    if (wnd == NULL) {
      dbg_printf("Didn't find wnd (tried FindWindowA and GetConsoleWindow)\n");
    }
  }
  return wnd;
}

BOOL LaunchedFromConsole(void) {
  if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
    return FALSE;
  } else {
    return TRUE;
  }
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
  kExitCode result;
  size_t i;
  uint32_t existing_originals_mask;
  uint32_t existing_replacements_mask;

  ExistMaskForLockedFiles(args, &existing_originals_mask);
  ExistMaskForReplacements(args, &existing_replacements_mask);

  /* We were asked to move files into place and none of them exist.
     Exit returning success. */
  if (args->move_suffix != NULL && existing_replacements_mask == 0) {
    return (kSuccessNoReplacements);
  }

  result = EnumerateLockingProcesses(args, state);
  if (result != kSuccess) {
    return (result);
  }

  for (i = 0; i < state->num_locking_processes; ++i) {
    uint32_t conflicts = 0x1010101;
    if (conflicts) {
      if (args->interface_style == kInterfaceGUI) {
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

void InterfaceStateInit(InterfaceState* p_if_state) {
  p_if_state->n_secs = 0.0f;

  /* http://www.codeproject.com/Articles/3537/Loading-JPG-PNG-resources-using-GDI
     http://stackoverflow.com/questions/9240188/how-to-load-a-custom-binary-resource-in-a-vc-static-library-as-part-of-a-dll
  */
  p_if_state->splash_jpg = FindResource(NULL, MAKEINTRESOURCE(IDC_SPLASH_JPG),
                                        MAKEINTRESOURCE(RT_RCDATA));
  p_if_state->splash_jpg_bitmap =
      LoadBitmap(NULL, MAKEINTRESOURCE(IDC_SPLASH_JPG));
  dbg_printf("splash_jpg %p, splash_jpg_bitmap %p\n", p_if_state->splash_jpg,
             p_if_state->splash_jpg_bitmap);
}

int main(int argc, char* argv[]) {
  Arguments args;
  GlobalState state;
  InterfaceState if_state;
  state.num_locking_processes = 0;
  state.p_locking_processes = NULL;

  state.appHWnd = FindConsoleHandle();

  InterfaceStateInit(&if_state);

  kExitCode args_result = ParseArguments(argc, argv, &args);
  if (args_result == kUserNoFiles) {
    PrintHelp();
    fprintf(stderr, "\nError: No filename(s) specified.");
    return (kUserNoFiles);
  } else if (args_result == kUserTooManyFiles) {
    fprintf(stderr, "\nError: Too many filenames specified, max is 32.\n");
    return (kUserTooManyFiles);
  } else if (args_result != kSuccess) {
    return (args_result);
  }

  if (args.action == kActionHelpOnly) {
    PrintHelp();
    return (kSuccess);
  }

  do {
    kExitCode update_result = UpdateState(&args, &state);
    if (update_result == kFailEnumProc) {
      fprintf(stderr, "\nError: Failed to enumerate processes.\n");
      return (kFailEnumProc);
    } else if (update_result == kSuccessNoReplacements) {
      dbg_printf("Success: No replacement files found.\n");
      return (kSuccess);
    }

  } while (1);

  return (kSuccess);
}
