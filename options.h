// Handles command line options
#ifndef _OPTIONS_H_
#define _OPTIONS_H_
#include <stdbool.h>

typedef struct {
  bool indexOption;
  bool longOption;
  bool recursiveOption;
} Options;

// Parse the command line arguments and set a struct specifying the enabled options
void Options_parseOptions(int argv, char* argc[], Options* options);

#endif
