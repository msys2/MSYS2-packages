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

//#define WIN32_MEAN_AND_LEAN

#include <windows.h>
#include <psapi.h>
#include <shellapi.h>
#include <gdiplus.h>
#include <process.h>
#include <winuser.h>

#include <stdio.h>
#include <conio.h>
#include <stdint.h>

#include "resource.h"

typedef enum {

  /* For conflict detection. */
  kActionUnspeficied,
  /* --help */
  kActionHelpOnly,
  /* --query */
  kActionQuery,
  /* --observe, --ask, --force */
  kActionUnlocker,

} kAction;

typedef enum {

  /* For conflict detection. */
  kUnlockUnknown,
  /* --observe */
  kUnlockObserve,
  /* --ask */
  kUnlockAsk,
  /* --force */
  kUnlockForce,

} kUnlockMode;

typedef enum {

  kInterfaceStyleNone = 0,
  /* --console */
  kInterfaceStyleTUI = (1<<0),
  /* --gui */
  kInterfaceStyleGUI = (1<<1),

} kInterfaceStyle;

typedef enum {

  /* All is good */
  kSuccess = 0,
  /* Asked to move files (--move-, but the replacements
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
  kFailGdiplusStartup,
  kFailGuiSplashGfxInit,
  kFailTuiNYI,

} kExitCode;

typedef struct {
  DWORD id;
  char* name;
  uint32_t mask_locked_files;
} ProcessDetails;

typedef struct {
  ULONG_PTR gdi_plus_token;
  HWND h_splash_wnd;
  HRSRC splash_jpg_hrsrcs[IDC_SPLASH_COUNT];
  HGLOBAL splash_jpg_bufs[IDC_SPLASH_COUNT];
  GpBitmap* splash_jpg_bitmaps[IDC_SPLASH_COUNT];
} GraphicalInterfaceState;

typedef struct {
} TextualInterfaceState;

typedef struct {
  float n_secs;
  BOOL displayed_splash;
  BOOL use_gui;
  BOOL use_tui;
  HANDLE ui_thread;
  unsigned int ui_thread_id;
  HANDLE ui_thread_event;

  GraphicalInterfaceState gui;
  TextualInterfaceState tui;
} InterfaceState;

float kSplashTime = 1.0f / (float)IDC_SPLASH_COUNT;
int kSplashRateMsec = 1000 / IDC_SPLASH_COUNT;

#define TIMER_ID 1000

/* Global variables plus things
   that are updated dynamically */
typedef struct {
  InterfaceState if_state;

  // TODORMD :: Move this into GraphicalInterfaceState?
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
  kInterfaceStyle interface_style; /* Bitmask, can have 2 modes active at once */
  size_t num_filenames;
  char** p_filenames;

  /* Cached. */
  size_t strlen_longest_filename;
} Arguments;

enum {
  kProcessIncrement = 512,
//  kModulesIncrement = 512,
    kModulesIncrement = 4,
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

#define dbg_printf_GLE(DWORD_ERR_PTR, ...)                         \
  do {                                                             \
    char gle_buf[256];                                             \
    *DWORD_ERR_PTR = GetLastError();                               \
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,                     \
                   NULL, *DWORD_ERR_PTR,                           \
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),      \
                   gle_buf, sizeof(gle_buf), NULL);                \
    dbg_printf(__VA_ARGS__);                                       \
    dbg_printf("GetLastError()=%ld :: %s", *DWORD_ERR_PTR, gle_buf);\
  } while (0)

#define fatal_err_printf(...)                  \
  do {                                         \
      fprintf(stderr, "%s(): ", __FUNCTION__); \
      printf(__VA_ARGS__);                     \
      fflush(stdout);                          \
    } while (1)

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
  args->interface_style = kInterfaceStyleNone;
  if (LaunchedFromConsole()) {
    args->interface_style = kInterfaceStyleTUI;
  } else {
    args->interface_style = kInterfaceStyleGUI;
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
          args->interface_style = args->interface_style|kInterfaceStyleGUI;
        } else if (!strcmp("--tui", argv[argi])) {
          args->interface_style = args->interface_style|kInterfaceStyleTUI;
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
    if (num_modules > 2000) {
      int a = 1;
      a;
    }

    bytes_used = num_modules * sizeof(DWORD);
    modules = (HMODULE*)alloca(bytes_used);
    if (modules == NULL) {
      CloseHandle(proc_handle);
      return -ENOMEM;
    }

    result =
        EnumProcessModules(proc_handle, modules, bytes_used, &bytes_needed);

    if (result != FALSE) {
      if (bytes_used < bytes_needed) {
        /* Not all processes were enumerated. */
        result = FALSE;
      }
    }
    if (result == FALSE) {
      DWORD err;
      dbg_printf_GLE(&err, "EnumProcessModules(proc_handle=%p) failed\n", proc_handle);
      if (err == ERROR_PARTIAL_COPY) {
        return 0;
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
  *sets *(uint32_t*)p_outvalues to the bitmask of conflicting ids. */
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
/*    printf("\t%s\n", module_name); */
    for (fid = 0; fid < args->num_filenames; ++fid) {
      if (!strcmp(module_name, args->p_filenames[fid])) {
        *p_mask_locked |= (1 << fid);
        dbg_printf("locking %s", module_name);
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

/* NYI: https://processhacker.sourceforge.io/forums/viewtopic.php?f=8&t=1584
        https://msdn.microsoft.com/en-us/library/windows/desktop/aa813708.aspx
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

//INT_PTR CALLBACK
//DialogProc(HWND hwnd_dlg, UINT msg, WPARAM w_param, LPARAM l_param) {
//  GlobalState* state = (GlobalState*)l_param;
//  (void)state;
//  switch (msg) {
//    case WM_INITDIALOG: {
//      return TRUE;
//      break;
//    }

//    case WM_COMMAND: {
//      switch (w_param) {
//        case IDOK: {
//          EndDialog(hwnd_dlg, 0);
//          return TRUE;
//          break;
//        }
//      }
//      break;
//    }

//    case WM_SIZE: {
//      HWND h_edit;
//      RECT rc_client;

//      GetClientRect(hwnd_dlg, &rc_client);

//      h_edit = GetDlgItem(hwnd_dlg, IDC_UNLOCKERDIALOG);
//      SetWindowPos(h_edit, NULL, 0, 0, rc_client.right, rc_client.bottom,
//                   SWP_NOZORDER);
//      break;
//    }

//    case WM_CLOSE: {
//      EndDialog(hwnd_dlg, kUserExited);
//      return TRUE;
//      break;
//    }

//    default: { break; }
//  }

//  return FALSE;
//}

/* Runs on the main thread continually. */
kExitCode UpdateState(Arguments* args, GlobalState* state) {
  kExitCode result;
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

//  for (i = 0; i < state->num_locking_processes; ++i) {
//    uint32_t conflicts = 0x1010101;
//    if (conflicts) {
//      if (args->interface_style & kInterfaceStyleGUI) {
//        INT_PTR result =
//            DialogBoxParamA(NULL, MAKEINTRESOURCEA(IDC_UNLOCKERDIALOG),
//                            state->appHWnd, DialogProc, (LPARAM)state);
//        if (result == kUserExited) {
//          return (kUserExited);
//        }
//      }
//    }
//  }
  return (kSuccess);
}

kExitCode TextualInterfaceInit(Arguments* args, TextualInterfaceState* p_if_tui_state/*InterfaceState* p_if_state*/) {
  (void)args;
  (void)p_if_tui_state;
  if (TRUE) {
    dbg_printf("Error: TextualInterfaceInit NYI\n");
    return (kFailTuiNYI);
  }

  return (kSuccess);
}

kExitCode LoadBitmapFromResource(HRSRC* p_hrsrc, HGLOBAL* p_buf, GpBitmap** pp_bitmap, DWORD rcid) {
    *p_hrsrc = NULL;
    *p_buf = NULL;

    *p_hrsrc = FindResource(NULL, MAKEINTRESOURCE(rcid), MAKEINTRESOURCE(RT_RCDATA));
    if (*p_hrsrc == NULL) {
      return (kFailGuiSplashGfxInit);
    }

    DWORD size = SizeofResource(NULL, *p_hrsrc);
    if (!size) {
      return (kFailGuiSplashGfxInit);
    }
    *p_buf = GlobalAlloc(GMEM_MOVEABLE, size);
    if (*p_buf == NULL) {
      return (kFailGuiSplashGfxInit);
    }
    void* buf = GlobalLock(*p_buf);
    if (buf == NULL) {
      return (kFailGuiSplashGfxInit);
    }
    void* res_data = LockResource(LoadResource(NULL, *p_hrsrc));
    if (res_data == NULL) {
      return (kFailGuiSplashGfxInit);
    }
    memcpy(buf, res_data, size);
    IStream* stream = NULL;
    if (CreateStreamOnHGlobal(buf, FALSE, &stream) != S_OK) {
      return (kFailGuiSplashGfxInit);
    }
    GdipLoadImageFromStream(stream, pp_bitmap);
    return (kSuccess);
}

kExitCode GraphicalInterfaceInit(Arguments* args, GraphicalInterfaceState* p_if_gui_state/*InterfaceState* p_if_state*/) {
//    typedef struct {
//      ULONG_PTR gdi_plus_token;
//      HRSRC splash_jpg;
//      HBITMAP splash_jpg_bitmap;
//    } GraphicalInterfaceState;
    /* https://www.codeproject.com/Articles/3537/Loading-JPG-PNG-resources-using-GDI
       https://stackoverflow.com/questions/9240188/how-to-load-a-custom-binary-resource-in-a-vc-static-library-as-part-of-a-dll
       https://www.codeproject.com/Articles/15523/Own-thread-Win-splash-screen .. http://www.codeproject.com/Messages/3913756/Added-code-to-getImage-from-resource.aspx
       
    */
//  GraphicalInterfaceState* p_if_gui_state = &p_if_state->gui;
  (void)args;
  GdiplusStartupInput gdi_plus_statup_input;
  gdi_plus_statup_input.GdiplusVersion = 1;
  gdi_plus_statup_input.DebugEventCallback = NULL;
/*
  https://msdn.microsoft.com/en-us/library/windows/desktop/ms534067(v=vs.85).aspx
  
  Boolean value that specifies whether to suppress the GDI+ background thread. If you set this member to
  TRUE, GdiplusStartup returns (in its output parameter) a pointer to a hook function and a pointer to an
  unhook function. You must call those functions appropriately to replace the background thread. If you
  do not want to be responsible for calling the hook and unhook functions, set this member to FALSE.
  The default value is FALSE.
  
  .. SINCE I'LL IMPLEMENT MY OWN THREAD I PROBABLY WANT SuppressBackgroundThread = TRUE FINALLY.
*/
  gdi_plus_statup_input.SuppressBackgroundThread = FALSE;
  gdi_plus_statup_input.SuppressExternalCodecs = TRUE;
  
  GpStatus status = GdiplusStartup(&p_if_gui_state->gdi_plus_token, &gdi_plus_statup_input, NULL);
  if (status != Ok) {
    dbg_printf("Error: GdiplusStatup status %d", status);
    return (kFailGdiplusStartup);
  }
  for (size_t i = 0; i < IDC_SPLASH_COUNT; ++i) {
    kExitCode load_bitmap_exitcode = LoadBitmapFromResource (&p_if_gui_state->splash_jpg_hrsrcs[i], &p_if_gui_state->splash_jpg_bufs[i], &p_if_gui_state->splash_jpg_bitmaps[i], IDC_SPLASH_START + i);
    if (load_bitmap_exitcode != kSuccess) {
      return (load_bitmap_exitcode);
    }
    dbg_printf("splash_jpg[%02zd] = hrsrc=%p, buf=%p, bitmap=%p\n", i, p_if_gui_state->splash_jpg_hrsrcs[i], p_if_gui_state->splash_jpg_bufs[i], p_if_gui_state->splash_jpg_bitmaps[i]);
  }
  return (kSuccess);
}

LRESULT CALLBACK SplashWndProc(HWND hwnd_splash, UINT msg, WPARAM w_param, LPARAM l_param) {
  InterfaceState* p_if_state = (InterfaceState*)GetWindowLongPtr(hwnd_splash, -21/*GWL_USERDATA*/); // (InterfaceState*)l_param;
  
  if (p_if_state == NULL) {
    return DefWindowProc(hwnd_splash, msg, w_param, l_param);
  }
  // WM_NCCREATE, WM_NCCALCSIZE, WM_CREATE, WM_SIZE, WM_DESTROY
  switch (msg) {
    case WM_PAINT:
      {
        static int frame = 0;
        GpBitmap* bitmap = p_if_state->gui.splash_jpg_bitmaps[frame];
        if (frame < IDC_SPLASH_COUNT - 1) {
          ++frame;
        }
        if (bitmap) {
            GpGraphics* graphics;
            GdipCreateFromHWND(hwnd_splash, &graphics);
            REAL width, height;
            GdipGetImageDimension(bitmap, &width, &height);
            /* p_if_state->gui.gdi_plus_token */
            GdipDrawImageRectI(graphics, bitmap, 0, 0, width, height);

//            if ( pInstance->m_ProgressMsg.size() > 0 )
//            {
//                    Gdiplus::Font msgFont( L"Tahoma", 8, Gdiplus::UnitPixel );
//                    Gdiplus::SolidBrush msgBrush( static_cast<DWORD>(Gdiplus::Color::Black) );
//                    gdip.DrawString( pInstance->m_ProgressMsg.c_str(), -1, &msgFont, Gdiplus::PointF(2.0f, pInstance->m_pImage->GetHeight()-34.0f), &msgBrush );
//            }
            GdipDeleteGraphics(graphics);
        }
        ValidateRect(hwnd_splash, NULL);
        static int done_donate = 0;
        if (frame == IDC_SPLASH_COUNT - 1 && done_donate == 0) {
            done_donate = 1;
            ShellExecuteA(/*hwnd_splash*/NULL,
                         "open",
                         "https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=1390079",
                         NULL,
                         NULL,
                         SW_SHOW/*SW_SHOWNORMAL*/);
          }
        
        return 0;
        }
      break;
      case WM_TIMER:
      {
        InvalidateRect(p_if_state->gui.h_splash_wnd, NULL, TRUE);
        UpdateWindow(p_if_state->gui.h_splash_wnd);
      }
      break;
    }
  return DefWindowProc(hwnd_splash, msg, w_param, l_param);
}

kExitCode GraphicalSplashInitThread(InterfaceState* p_if_state) {

  WNDCLASS wndcls = {0};

  wndcls.style = CS_HREDRAW | CS_VREDRAW;
  wndcls.lpfnWndProc = SplashWndProc; 
  wndcls.hInstance = GetModuleHandle(NULL);
  wndcls.hCursor = LoadCursor(NULL, IDC_APPSTARTING);
  wndcls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wndcls.lpszClassName = L"SplashWnd";
  wndcls.hIcon = LoadIcon(wndcls.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

  if (!RegisterClass(&wndcls)) {
    if (GetLastError() != 0x00000582) // already registered)
    {
      OutputDebugString(L"Unable to register class SplashWnd\n");
      return 0;
    }
  }
  
  char name[] = "";
  // TODO :: Fix this.
  HWND h_parent_wnd = NULL;

  REAL width, height;
  GdipGetImageDimension(p_if_state->gui.splash_jpg_bitmaps[0], &width, &height);
  
  // try to find monitor where mouse was last time
  POINT point = { 0 };
  MONITORINFO mi = { sizeof(MONITORINFO), {0}, {0}, 0 };
  HMONITOR hMonitor = 0;
  RECT rcArea = { 0 };
  GetCursorPos( &point );

  hMonitor = MonitorFromPoint( point, MONITOR_DEFAULTTONEAREST );
  if ( GetMonitorInfo( hMonitor, &mi ) )
  {
    rcArea.left = ( mi.rcMonitor.right + mi.rcMonitor.left - (LONG)width ) / 2;
    rcArea.top = ( mi.rcMonitor.top + mi.rcMonitor.bottom - (LONG)height) / 2;
  }
  else
  {
    SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);
    rcArea.left = (rcArea.right + rcArea.left - (LONG)width)/2;
    rcArea.top = (rcArea.top + rcArea.bottom - (LONG)height)/2;
  }  

  p_if_state->gui.h_splash_wnd = CreateWindowExA(strlen(name)?0:WS_EX_TOOLWINDOW, "SplashWnd", name, 
       WS_CLIPCHILDREN|WS_POPUP, rcArea.left, rcArea.top, width, height, h_parent_wnd, NULL, wndcls.hInstance, NULL);

  SetWindowLongPtr(p_if_state->gui.h_splash_wnd, /*GWL_USERDATA*/ -21, (LONG_PTR)p_if_state);
  ShowWindow(p_if_state->gui.h_splash_wnd, SW_SHOWNOACTIVATE);

  MSG msg;
  PeekMessage(&msg, NULL, 0, 0, 0); // invoke creating message queue
  return (kSuccess);
}

kExitCode TextualSplashInitThread(InterfaceState* p_if_state) {
  return (kSuccess);
}

unsigned int __stdcall SplashThread(void* param) {
  InterfaceState* p_if_state = (InterfaceState*)param;
  (void)p_if_state;

  if (p_if_state->use_gui) {
    GraphicalSplashInitThread(p_if_state);
  }
  if (p_if_state->use_tui) {
    TextualSplashInitThread(p_if_state);
  }
  SetEvent(p_if_state->ui_thread_event);
  
  MSG msg;
  BOOL bRet;
  SetTimer(p_if_state->gui.h_splash_wnd, TIMER_ID, kSplashRateMsec, 0);
  while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
    TranslateMessage(&msg); 
    DispatchMessage(&msg);
  }

  return (0);
}

kExitCode InterfaceStateInit(Arguments* args, InterfaceState* p_if_state) {
  p_if_state->n_secs = 0.0f;

  kExitCode status = kSuccess;
  kExitCode status_tui = kSuccess;
  kExitCode status_gui = kSuccess;

  p_if_state->use_gui = FALSE;
  p_if_state->use_tui = FALSE;
  if (args->interface_style & kInterfaceStyleTUI) {
      status_tui = TextualInterfaceInit(args, &p_if_state->tui);
      if (status_tui == kSuccess) {
        p_if_state->use_tui = TRUE;
      }
  }
  if (args->interface_style & kInterfaceStyleGUI) {
      status_gui = GraphicalInterfaceInit(args, &p_if_state->gui);
      if (status_gui == kSuccess) {
        p_if_state->use_gui = TRUE;
      }
  }
  if (status_tui != kSuccess && status_gui != kSuccess) status = status_tui;

  p_if_state->ui_thread_event = CreateEvent(NULL, FALSE, FALSE, NULL);
  p_if_state->ui_thread = (HANDLE)_beginthreadex(NULL, 0, SplashThread, (void*)p_if_state, 0, &p_if_state->ui_thread_id);
  if (WaitForSingleObject(p_if_state->ui_thread_event, 5000) == INFINITE) {
          dbg_printf("ui_thread_event : WAIT_TIMEOUT\n");
  }

  return (status);
}

kExitCode TextualInterfaceExit(Arguments* args, TextualInterfaceState* p_if_tui_state) {
  (void)args;
  (void)p_if_tui_state;
  return (kSuccess);
}

kExitCode GraphicalInterfaceExit(Arguments* args, GraphicalInterfaceState* p_if_gui_state) {
  (void)args;
  (void)p_if_gui_state;
  return (kSuccess);
}

kExitCode InterfaceStateExit(Arguments* args, InterfaceState* p_if_state) {
    /* Tell the UI thread to die .. */
    if (args->interface_style & kInterfaceStyleTUI) {
      TextualInterfaceExit(args, &p_if_state->tui);
    }
    if (args->interface_style & kInterfaceStyleGUI) {
      GraphicalInterfaceExit(args, &p_if_state->gui);
    }
    return (kSuccess);
}

void CleanupAndExit(Arguments* args, GlobalState* state, kExitCode exit_code) {
  kExitCode cleanup_exit_code = InterfaceStateExit(args, &state->if_state);
  if (cleanup_exit_code) {
    fatal_err_printf("Error (on-exit, ignored): InterfaceStateExit failed.");
  }
  exit(exit_code);
}

int main(int argc, char* argv[]) {
  Arguments args;
  GlobalState state;
  state.num_locking_processes = 0;
  state.p_locking_processes = NULL;

  state.appHWnd = FindConsoleHandle();
  
  // TODO :: This should check argv[0] and behave differently for each of msys2, mingw32 and mingw64.
  // TODO :: It should also interpret the filenames as relative paths from argv[0].
  char* default_args[] = {"msys2_updater.exe", "--gui", "C:\\msys64\\usr\\bin\\msys-2.0.dll", "C:\\msys64\\usr\\bin\\bash.exe"};
  if (argc == 1) {
      argc = sizeof(default_args)/sizeof(default_args[0]);
      argv = default_args;
  }

  kExitCode exit_code = ParseArguments(argc, argv, &args);
  if (exit_code == kUserNoFiles) {
    PrintHelp();
    fprintf(stderr, "\nError: No filename(s) specified.");
    return (exit_code);
  } else if (exit_code == kUserTooManyFiles) {
    fprintf(stderr, "\nError: Too many filenames specified, max is 32.\n");
    return (exit_code);
  } else if (exit_code != kSuccess) {
    return (exit_code);
  }
  exit_code = InterfaceStateInit(&args, &state.if_state);
  
  if (exit_code != kSuccess) {
    CleanupAndExit(&args, &state, exit_code);
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
