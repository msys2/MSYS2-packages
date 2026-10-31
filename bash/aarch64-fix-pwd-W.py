"""Fix `pwd -W` on aarch64-pc-cygwin.

The MSYS2 -W patch (0006-bash-4.3-add-pwd-W-option.patch) does:

    buffer  = xmalloc (PATH_MAX);
    wbuffer = xmalloc (PATH_MAX);
    directory = getcwd (buffer, PATH_MAX);
    cygwin_conv_path (CCP_POSIX_TO_WIN_A|CCP_ABSOLUTE, buffer, wbuffer, PATH_MAX+1);

wbuffer is PATH_MAX bytes but cygwin_conv_path is told it is PATH_MAX+1,
neither buffer is zeroed, and getcwd's return value is never checked.  On
AArch64 the converted path comes back unterminated, so `pwd -W` prints the
path with whatever follows it on the heap appended, e.g.

    C:/fbwork/-W                 C:/fbwork/repo/.git/-v

git-for-windows overrides pwd with a wrapper that calls `builtin pwd -W`, and
git-filter-branch builds its temp directory from it:

    tempdir="$(cd "$tempdir"; pwd)"

so every --tree-filter / --index-filter run died on a corrupted path.
"""
import io
import sys

PATH = "builtins/cd.def"

OLD = """    buffer = xmalloc (PATH_MAX);
    wbuffer = xmalloc (PATH_MAX);
    directory = getcwd (buffer, PATH_MAX);
    cygwin_conv_path (CCP_POSIX_TO_WIN_A|CCP_ABSOLUTE, buffer, wbuffer, PATH_MAX+1);"""

NEW = """    buffer = xmalloc (PATH_MAX + 1);
    wbuffer = xmalloc (PATH_MAX + 1);
    memset (buffer, 0, PATH_MAX + 1);
    memset (wbuffer, 0, PATH_MAX + 1);
    directory = getcwd (buffer, PATH_MAX);
    if (directory == 0)
      {
        free (buffer);
        free (wbuffer);
        builtin_error ("getcwd: cannot access parent directories");
        return (EXECUTION_FAILURE);
      }
    if (cygwin_conv_path (CCP_POSIX_TO_WIN_A|CCP_ABSOLUTE, buffer, wbuffer,
                          PATH_MAX) != 0)
      {
        free (buffer);
        free (wbuffer);
        builtin_error ("pwd: cannot convert path");
        return (EXECUTION_FAILURE);
      }
    wbuffer[PATH_MAX] = 0;"""


def main():
    text = io.open(PATH, encoding="utf-8", newline="").read()
    if NEW in text:
        sys.stderr.write("pwd -W fix: already applied\n")
        return 0
    if OLD not in text:
        sys.stderr.write("pwd -W fix: anchor not found in %s\n" % PATH)
        return 1
    io.open(PATH, "w", encoding="utf-8", newline="").write(text.replace(OLD, NEW, 1))
    sys.stderr.write("pwd -W fix applied to %s\n" % PATH)
    return 0


if __name__ == "__main__":
    sys.exit(main())
