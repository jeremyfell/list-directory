#include "options.h"
#include "files.h"

int main(int argc, char* argv[]) {
  // Get the options from the command line arguments
  Options options;
  Options_parseOptions(argc, argv, &options);

  // Get the files/directories from the command line arguments
  int filenamesLength;
  char** filenames;
  Files_getFilenames(argc, argv, &filenamesLength, &filenames);

  // List the specified files and directories
  Files_list(filenamesLength, filenames, &options);

  return 0;
}
