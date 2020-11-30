// Handles files
#ifndef _FILES_H_
#define _FILES_H_
#include "options.h"

// Gets the array of filenames from the command line arguments
// Sets the length of the filenames array and sets the pointer to the filenames array
void Files_getFilenames(int argc, char* argv[], int* filenamesLengthAddress, char*** filenamesAddress);

// List all files and directories from the provided filename arguments
void Files_list(int filenamesLength, char** filenames, Options* pOptions);

#endif
