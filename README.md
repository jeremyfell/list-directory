List Directory
==============
A simplified implementation of the Unix ls program, to list out files and directories on a command line terminal.

To use, first open the repo and run `make` to build the program.

To run, use `./list <options> <list of files/directories>`

e.g. `./list -R dir1 file1 dir2`

If no files or directories are provided, the current directory will be printed.

Options
-------
- `-l` prints out additional file info, including file permissions, the number of links to the file, the owner, the group, the file size, and the last modification time
- `-i` prints out file inode numbers
- `-R` recursively prints out all subdirectories
- Multiple options can be used, in any order. e.g `-iRl`
