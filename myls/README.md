# NetBSD `ls(1)` Midterm Project

## Student Information

- **Student:** Le Van Tri
- **Student ID:** 24IT282
- **Project:** Implementation of `ls(1)` on NetBSD
- **Operating System:** NetBSD 10.1 (amd64)
- **Language:** C (C99)
- **Compiler:** NetBSD `cc`
- **Build Tool:** BSD `make`
- **Repository:** https://github.com/LeVanTri2006/LeVanTri_24IT282_midterm

## Project Description

This project implements a simplified version of the NetBSD `ls(1)` command in C.

The program lists files and directories, retrieves file metadata, handles symbolic links, supports command-line options, sorts entries, displays long-format information, and recursively traverses subdirectories.

The source code is organized into separate modules for option parsing, directory reading, sorting, output formatting, and utility functions. The project was compiled and tested on NetBSD 10.1 using the system C compiler and BSD `make`.

## Supported Options

The program implements the following command-line options:

- `-A` : Show almost all entries, excluding `.` and `..`
- `-a` : Show all entries, including hidden files, `.` and `..`
- `-c` : Use file status change time
- `-d` : List directories as plain files instead of listing their contents
- `-F` : Append file type indicators, such as `/`, `*`, and `@`
- `-f` : Disable sorting
- `-h` : Display sizes in human-readable format
- `-i` : Display inode numbers
- `-k` : Display sizes in kilobytes
- `-l` : Use long listing format
- `-n` : Use numeric user and group IDs in long format
- `-q` : Replace non-printable characters with `?`
- `-R` : Recursively list subdirectories
- `-r` : Reverse the selected sort order
- `-S` : Sort entries by file size, largest first
- `-s` : Display allocated block counts
- `-t` : Sort entries by time
- `-u` : Use last access time
- `-w` : Print file names without replacing non-printable characters

Options that select time fields, size formats, or output formats may be combined where supported by the implementation.

## Project Structure

```text
myls/
├── Makefile
├── README.md
├── .gitignore
├── include/
│   ├── ls.h
│   ├── options.h
│   ├── list.h
│   ├── display.h
│   ├── sort.h
│   └── util.h
└── src/
    ├── main.c
    ├── options.c
    ├── list.c
    ├── display.c
    ├── sort.c
    └── util.c
```

### Module Responsibilities

| Module      | Responsibility                                                          |
| ----------- | ----------------------------------------------------------------------- |
| `main.c`    | Program entry point and command-line operand processing                 |
| `options.c` | Parse command-line options using `getopt()`                             |
| `list.c`    | Read directories, collect file metadata, and handle symbolic links      |
| `sort.c`    | Sort entries by name, size, or time                                     |
| `display.c` | Display file names, metadata, block counts, and long-format output      |
| `util.c`    | Format file permissions, human-readable sizes, and file type indicators |
| `include/`  | Shared data structures, option definitions, and function declarations   |

## Build Instructions

Build the program on NetBSD 10.1 from the `myls` project directory.

Clean previous build artifacts:

```sh
make MAKEOBJDIR=. clean
```

Compile the program:

```sh
make MAKEOBJDIR=.
```

The build creates the executable `myls` and object files under `obj/`.

**Note:** `MAKEOBJDIR=.` is used with the current Makefile to ensure that BSD `make` runs the build with the project directory as its working directory.

## Running the Program

Run the default listing:

```sh
./myls
```

List all entries, including hidden files:

```sh
./myls -a
```

Display detailed file information:

```sh
./myls -l
```

Display numeric user and group IDs:

```sh
./myls -n
```

Sort entries by file size:

```sh
./myls -S
```

Recursively list subdirectories:

```sh
./myls -R
```

List a specific file or directory:

```sh
./myls alpha.txt
./myls -d subdir
```

Clean build artifacts:

```sh
make MAKEOBJDIR=. clean
```

## Testing

The program was compiled and executed on NetBSD 10.1.

A test directory was created containing regular files, a hidden file, an executable file, a symbolic link, a subdirectory, and a larger test file.

The following groups of tests were performed:

- Basic listing and hidden-file handling: default, `-a`, and `-A`
- Long-format and size output: `-l`, `-n`, and `-lh`
- File metadata: `-i` and `-s`
- Sorting: `-S`, `-t`, `-r`, and `-Sr`
- File type indicators and symbolic links: `-F` and `-l`
- Recursive traversal and directory operands: `-R` and `-d`
- Error handling for non-existent operands

Selected outputs were compared against `/bin/ls` using `diff`. The tested cases matched in the comparisons performed. Long-format outputs still differ in column spacing, although the compared content matched after whitespace normalization.

## `.gitignore`

The `.gitignore` file excludes the compiled executable, object files, core dumps, temporary files, editor configuration, and other unnecessary artifacts from version control.

This keeps the repository focused on the source code and project documentation rather than generated build files.

## References

- NetBSD 10.1 `ls(1)` manual page
- POSIX command-line and file-system interfaces
- NetBSD system headers and C library documentation
