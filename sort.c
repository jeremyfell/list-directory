#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include "sort.h"

// Lexicographical comparison function
static int compare(const void* a, const void* b) {
  return strncmp(*(char**) a, *(char**) b, PATH_MAX);
}

// Lexicographically sorts an array of strings in place
void Sort_lexicographicalSort(int length, char** strings) {
  qsort(strings, length, sizeof(char*), compare);
  return;
}
