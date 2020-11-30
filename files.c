#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "files.h"
#include "options.h"
#include "sort.h"
#include "helpers.h"

// Provides information about a group of files for print formatting
typedef struct {
  int maxIdDigits; // The max # of digits needed to represent any file's ID in the group
  int maxLinksDigits; // The max # of digits needed to represent any file's hard link count
  int maxSizeDigits; // The max # of digits needed to represent any file's size in the group
  int maxUserLetters; // The max # of letters needed to represent any file's user name in the group
  int maxGroupLetters; // The max # of letters needed to represent any file's group name in the group
  bool hasSpecialCharacters; // At least one file contains special characters
} FileGroupInfo;

// Gets a file or directory path by combining its name with the parent directory path
// Returned string must eventually be freed
static char* getPath(char* filename, char* directoryPath) {
  char* path = malloc(PATH_MAX);

  strcpy(path, directoryPath);

  int directoryLength = strlen(directoryPath);
  if (directoryPath[directoryLength - 1] != '/') {
    strcat(path, "/");
  }

  strcat(path, filename);

  return path;
}

// Gets information about a group of files and sets a struct with this info
// Pass directoryPath as NULL if all provided filenames are already valid paths
static void getFileGroupInfo(int filenamesLength, char** filenames, char* directoryPath, FileGroupInfo* pFileGroupInfo) {
  long maxId = 0;
  long maxLinks = 0;
  long maxSize = 0;
  int maxUserLetters = 0;
  int maxGroupLetters = 0;
  bool hasSpecialCharacters = false;

  for (int i = 0; i < filenamesLength; i++) {
    struct stat statBuffer;

    if (directoryPath == NULL) {
      lstat(filenames[i], &statBuffer);
    } else {
      char* filePath = getPath(filenames[i], directoryPath);
      lstat(filePath, &statBuffer);
      free(filePath);
      filePath = NULL;
    }

    char* specialCharacters = " !$^&()'`\"";
    char* findSpecialCharacters = strpbrk(filenames[i], specialCharacters);

    if (findSpecialCharacters != NULL) {
      hasSpecialCharacters = true;
    }

    if (statBuffer.st_ino > maxId) {
      maxId = statBuffer.st_ino;
    }

    if (statBuffer.st_nlink > maxLinks) {
      maxLinks = statBuffer.st_nlink;
    }

    if (statBuffer.st_size > maxSize) {
      maxSize = statBuffer.st_size;
    }

    char* userName = Helpers_getUserName(statBuffer.st_uid);
    int userNameLength = strlen(userName);
    if (userNameLength > maxUserLetters) {
      maxUserLetters = userNameLength;
    }

    char* groupName = Helpers_getGroupName(statBuffer.st_gid);
    int groupNameLength = strlen(groupName);
    if (groupNameLength > maxGroupLetters) {
      maxGroupLetters = groupNameLength;
    }
  }

  if (maxId == 0) {
    pFileGroupInfo->maxIdDigits = 1;
  } else {
    pFileGroupInfo->maxIdDigits = floor(log10(maxId)) + 1;
  }

  if (maxLinks == 0) {
    pFileGroupInfo->maxLinksDigits = 1;
  } else {
    pFileGroupInfo->maxLinksDigits = floor(log10(maxLinks)) + 1;
  }

  if (maxSize == 0) {
    pFileGroupInfo->maxSizeDigits = 1;
  } else {
    pFileGroupInfo->maxSizeDigits = floor(log10(maxSize)) + 1;
  }

  pFileGroupInfo->hasSpecialCharacters = hasSpecialCharacters;

  pFileGroupInfo->maxUserLetters = maxUserLetters;
  pFileGroupInfo->maxGroupLetters = maxGroupLetters;

  return;
}

// Separates files and directories into two different arrays
// Sets the length and array pointer for both arrays
// directories and files arrays must eventually be freed
static void separateFilesAndDirectories(int filenamesLength, char** filenames, int* pDirectoriesLength, char*** pDirectories, int* pFilesLength, char*** pFiles, Options* pOptions) {
  int status = 0;
  int filesLength = 0;
  int directoriesLength = 0;
  char** files = malloc(sizeof(char*) * filenamesLength);
  char** directories = malloc(sizeof(char*) * filenamesLength);

  for (int i = 0; i < filenamesLength; i++) {
    struct stat statBuffer;

    status = lstat(filenames[i], &statBuffer);

    // Error handling
    if (status == -1) {
      if (errno == ENAMETOOLONG) {
        printf("list: cannot access '%s': File name too long\n", filenames[i]);
      } else {
        printf("list: cannot access '%s': No such file or directory\n", filenames[i]);
      }
      continue;
    }

    // Handle directories
    if (S_ISDIR(statBuffer.st_mode)) {
      directories[directoriesLength] = malloc(PATH_MAX);
      strcpy(directories[directoriesLength], filenames[i]);
      directoriesLength++;
      continue;
    }

    // Handle files
    files[filesLength] = malloc(PATH_MAX);
    strcpy(files[filesLength], filenames[i]);
    filesLength++;
  }

  *pDirectoriesLength = directoriesLength;
  *pDirectories = directories;
  *pFilesLength = filesLength;
  *pFiles = files;
  return;
}


// Print the file name
// Prints using single quotes when the file name contains special characters
// Prints using double quotes when the file name contains single quotes
static void printFilename(char* filename, bool addExtraSpace) {
  char singleQuote = '\'';
  char* findSingleQuote = strchr(filename, singleQuote);

  if (findSingleQuote == NULL) {
    char* specialCharacters = " !$^&()`\"";
    char* findSpecialCharacters = strpbrk(filename, specialCharacters);

    if (findSpecialCharacters == NULL) {
      if (addExtraSpace) {
        printf(" %s", filename);
      } else {
        printf("%s", filename);
      }
    } else {
      printf("'%s'", filename);
    }
  } else {
    printf("\"%s\"", filename);
  }

  return;
}

// Prints out the details of a file or directory file
static void printFileDetails(char* filename, char* filePath, struct stat statBuffer, FileGroupInfo* pInfo, Options* pOptions) {
  char modeBuffer[MODE_STRING_LENGTH];
  char dateBuffer[DATE_STRING_LENGTH];
  char symbolicBuffer[PATH_MAX];

  // If the -i option is used, print the index number of the file
  if (pOptions->indexOption) {
    printf("%*ld ", pInfo->maxIdDigits, statBuffer.st_ino);
  }

  // If the -l option is used, Print the mode, # of hard links, user, group,
  // size, and last modified date of the file
  if (pOptions->longOption) {
    printf(
      "%s %*ld %-*s %-*s %*ld %s ",
      Helpers_parseMode(statBuffer.st_mode, modeBuffer),
      pInfo->maxLinksDigits,
      statBuffer.st_nlink,
      pInfo->maxUserLetters,
      Helpers_getUserName(statBuffer.st_uid),
      pInfo->maxGroupLetters,
      Helpers_getGroupName(statBuffer.st_gid),
      pInfo->maxSizeDigits,
      statBuffer.st_size,
      Helpers_parseDate(statBuffer.st_mtime, dateBuffer)
    );
  }

  // Print the file name
  // Prints using single quotes when the file name contains special characters
  // Prints using double quotes when the file name contains single quotes
  printFilename(filename, pInfo->hasSpecialCharacters);

  // If the file is a symbolic link, print the file name that it points to
  if (pOptions->longOption && S_ISLNK(statBuffer.st_mode)) {
    int linkLength = readlink(filePath, symbolicBuffer, PATH_MAX);

    if (linkLength != -1) {
      int terminateIndex = (linkLength < PATH_MAX) ? linkLength : PATH_MAX - 1;
      symbolicBuffer[terminateIndex] = '\0';
      printf(" -> ");
      printFilename(symbolicBuffer, false);
    }
  }

  printf("\n");
  return;
}

// Iterates through a directory, calling a function for every file/subdirectory
// This called function will be passed the current index, the current directory entry, and pArgument
// Skips the '.' and '..' directories
// directoryStream must be an open directory stream
// Subsequent calls to iterateDirectory must first call rewinddir on the directory stream
// Returns the size of the directory (# of files/subdirectories inside)
typedef void (*ITERATE_FUNCTION)(int index, struct dirent* pDirectoryEntry, void* pArgument);
static int iterateDirectory(DIR* directoryStream, ITERATE_FUNCTION pIterateFunction, void* pArgument) {
  int length = 0;
  struct dirent* pDirectoryEntry;

  while (true) {
    pDirectoryEntry = readdir(directoryStream);

    if (pDirectoryEntry == NULL) {
      break;
    }

    if (pDirectoryEntry->d_name[0] == '.') {
      continue;
    }

    if (pIterateFunction != NULL) {
      (*pIterateFunction)(length, pDirectoryEntry, pArgument);
    }

    length++;
  }

  return length;
}

// Records the filename from a directory entry into a string array
// Passed as an argument to iterateDirectory
static void recordFilename(int index, struct dirent* pDirectoryEntry, void* pArgument) {
  char** filenames = pArgument;

  filenames[index] = malloc(NAME_MAX);
  strncpy(filenames[index], pDirectoryEntry->d_name, NAME_MAX);

  return;
}

// Prints out the contents of a directory
// The calling function is responsible for printing out the directory name if needed
// If -R option is set, also recursively prints all subdirectories
static void printDirectory(char* directoryPath, Options* pOptions) {
  int status = 0;
  DIR* directoryStream = opendir(directoryPath);

  if (directoryStream == NULL) {
    printf("list: cannot open directory '%s': Permission denied\n", directoryPath);
    return;
  }

  // Count the number of files/subdirectories in the directory
  int filenamesLength = iterateDirectory(directoryStream, NULL, NULL);
  int directoriesLength = 0;

  char** filenames = malloc(sizeof(char*) * filenamesLength);
  char** directories = NULL;

  if (pOptions->recursiveOption) {
    directories = malloc(sizeof(char*) * filenamesLength);
  }

  rewinddir(directoryStream);

  // Get the filenames of all files/subdirectories in the directory
  iterateDirectory(directoryStream, recordFilename, filenames);

  Sort_lexicographicalSort(filenamesLength, filenames);

  // Get information about the group of files/subdirectories
  FileGroupInfo fileGroupInfo;
  getFileGroupInfo(filenamesLength, filenames, directoryPath, &fileGroupInfo);

  for (int i = 0; i < filenamesLength; i++) {
    struct stat statBuffer;

    // Get the path for each file/subdirectory
    char* filePath = getPath(filenames[i], directoryPath);

    lstat(filePath, &statBuffer);

    // Print out the details of the file according to what options are set
    printFileDetails(filenames[i], filePath, statBuffer, &fileGroupInfo, pOptions);

    free(filePath);
    filePath = NULL;


    // If the -R option is set, create an array of all subdirectories
    if (pOptions->recursiveOption && S_ISDIR(statBuffer.st_mode)) {
      directories[directoriesLength] = filenames[i];
      directoriesLength++;
    } else {
      free(filenames[i]);
    }
    filenames[i] = NULL;
  }

  free(filenames);
  filenames = NULL;

  status = closedir(directoryStream);
  directoryStream = NULL;

  if (status == -1) {
    printf("list: could not close directory\n");
  }

  // If -R option is set, recursively print all subdirectories
  if (pOptions->recursiveOption) {
    for (int i = 0; i < directoriesLength; i++) {
      char* childDirectoryPath = getPath(directories[i], directoryPath);

      printf("\n%s:\n", childDirectoryPath);
      printDirectory(childDirectoryPath, pOptions);

      free(childDirectoryPath);
      childDirectoryPath = NULL;
    }

    Helpers_freeStringArray(directoriesLength, directories);
    directories = NULL;
  }

  return;
}

// Gets the array of filenames from the command line arguments
// Sets the length of the filenames array and sets the pointer to the filenames array
void Files_getFilenames(int argc, char* argv[], int* filenamesLengthAddress, char*** filenamesAddress) {
  char** filenames = argv + 1;
  int filenamesLength = argc - 1;

  // Move array pointer to the first file name argument
  while (filenamesLength && filenames[0][0] == '-') {
    filenames++;
    filenamesLength--;
  }

  *filenamesLengthAddress = filenamesLength;
  *filenamesAddress = filenames;
  return;
}

// List all files and directories from the provided filename arguments
void Files_list(int filenamesLength, char** filenames, Options* pOptions) {
  // Use current directory as default if no file arguments are provided
  char* defaultFilenames[] = {"."};
  if (filenamesLength == 0) {
    filenames = defaultFilenames;
    filenamesLength = 1;
  }

  // Separate the filename arguments into directories and files
  int directoriesLength = 0;
  int filesLength = 0;
  char** directories;
  char** files;
  separateFilesAndDirectories(
    filenamesLength, filenames,
    &directoriesLength, &directories,
    &filesLength, &files,
    pOptions
  );

  // Sort and print all the files
  if (filesLength > 0) {
    FileGroupInfo fileGroupInfo;
    getFileGroupInfo(filesLength, files, NULL, &fileGroupInfo);

    Sort_lexicographicalSort(filesLength, files);

    for (int i = 0; i < filesLength; i++) {
      struct stat statBuffer;
      lstat(files[i], &statBuffer);
      printFileDetails(files[i], files[i], statBuffer, &fileGroupInfo, pOptions);
    }
  }

  Helpers_freeStringArray(filesLength, files);
  files = NULL;

  // Sort and print all the directories
  if (directoriesLength > 0) {
    if (filenamesLength == 1) {
      if (pOptions->recursiveOption) {
        printf("%s:\n", directories[0]);
      }

      printDirectory(directories[0], pOptions);
    } else {
      Sort_lexicographicalSort(directoriesLength, directories);

      for (int i = 0; i < directoriesLength; i++) {
        if (i != 0 || filesLength != 0) {
          printf("\n");
        }

        printf("%s:\n", directories[i]);

        printDirectory(directories[i], pOptions);
      }
    }
  }

  Helpers_freeStringArray(directoriesLength, directories);
  directories = NULL;

  return;
}
