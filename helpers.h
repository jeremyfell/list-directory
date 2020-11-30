// Provides several helper functions
#ifndef _HELPERS_H_
#define _HELPERS_H_

#define MODE_STRING_LENGTH 11
#define DATE_STRING_LENGTH 18
#define EMPTY_STRING ""

// Parse the mode integer into a formatted string
// formattedMode must be length 11 (to hold 10 chars)
char* Helpers_parseMode(mode_t mode, char* formattedMode);

// Parse the timestamp into a formatted string
// formattedDate must be length 18 (to hold 17 chars)
char* Helpers_parseDate(time_t timestamp, char* dateBuffer);

// Get the group name from the group ID
char* Helpers_getGroupName(gid_t gid);

// Get the user name from the user ID
char* Helpers_getUserName(uid_t uid);

// Frees an array of strings
void Helpers_freeStringArray(int length, char** stringArray);

#endif
