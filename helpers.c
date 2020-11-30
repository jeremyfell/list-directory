#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include "helpers.h"

// Parse the mode integer into a formatted string
// formattedMode must be length 11 (to hold 10 chars)
char* Helpers_parseMode(mode_t mode, char* formattedMode) {
  if (S_ISDIR(mode)) {
    formattedMode[0] = 'd';
  } else if (S_ISLNK(mode)) {
    formattedMode[0] = 'l';
  } else {
    formattedMode[0] = '-';
  }

  formattedMode[1] = (mode & S_IRUSR) ? 'r' : '-';
  formattedMode[2] = (mode & S_IWUSR) ? 'w' : '-';
  formattedMode[3] = (mode & S_IXUSR) ? 'x' : '-';
  formattedMode[4] = (mode & S_IRGRP) ? 'r' : '-';
  formattedMode[5] = (mode & S_IWGRP) ? 'w' : '-';
  formattedMode[6] = (mode & S_IXGRP) ? 'x' : '-';
  formattedMode[7] = (mode & S_IROTH) ? 'r' : '-';
  formattedMode[8] = (mode & S_IWOTH) ? 'w' : '-';
  formattedMode[9] = (mode & S_IXOTH) ? 'x' : '-';
  formattedMode[10] = '\0';
  return formattedMode;
}

// Parse the timestamp into a formatted string
// formattedDate must be length 18 (to hold 17 chars)
char* Helpers_parseDate(time_t timestamp, char* dateBuffer) {
  struct tm* time = localtime(&timestamp);
  strftime(dateBuffer, DATE_STRING_LENGTH, "%b %e %Y %H:%M", time);
  return dateBuffer;
}

// Get the group name from the group ID
// Adapted from the provided infodemo.c
char* Helpers_getGroupName(gid_t gid) {
    struct group* pGroup = getgrgid(gid);

    if (pGroup == NULL) {
      return EMPTY_STRING;
    }

    return pGroup->gr_name;
}

// Get the user name from the user ID
// Adapted from the provided infodemo.c
char* Helpers_getUserName(uid_t uid) {
    struct passwd* password = getpwuid(uid);

    if (password == NULL) {
      return EMPTY_STRING;
    }

    return password->pw_name;
}

// Frees an array of strings
void Helpers_freeStringArray(int length, char** stringArray) {
  for (int i = 0; i < length; i++) {
    free(stringArray[i]);
  }
  free(stringArray);

  return;
}
