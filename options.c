#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "options.h"

static void repeatedOptionError() {
  printf("list: cannot specify an option multiple times\n");
  exit(1);
  return;
}

static void invalidOptionsError() {
  printf("list: invalid options provided\n");
  exit(1);
  return;
}

// Parse the command line arguments and set a struct specifying the enabled options
void Options_parseOptions(int argc, char* argv[], Options* pOptions) {
  pOptions->indexOption = false;
  pOptions->longOption = false;
  pOptions->recursiveOption = false;

  // Check if any options were provided
  for (int i = 1; i < argc; i++) {
    char* optionsString = argv[i];

    // Stop when a non-option argument is encountered
    if (optionsString[0] != '-') {
      break;
    }

    if (optionsString[1] == '\0') {
      invalidOptionsError();
      return;
    }

    optionsString++;
    char optionLetter = *optionsString;

    while (optionLetter != '\0') {
      if (optionLetter == 'i') {
        if (pOptions->indexOption) {
          repeatedOptionError();
        } else {
          pOptions->indexOption = true;
        }
      } else if (optionLetter == 'l'){
        if (pOptions->longOption) {
          repeatedOptionError();
        } else {
          pOptions->longOption = true;
        }
      } else if (optionLetter == 'R') {
        if (pOptions->recursiveOption) {
          repeatedOptionError();
        } else {
          pOptions->recursiveOption = true;
        }
      } else {
        invalidOptionsError();
      }

      optionsString++;
      optionLetter = *optionsString;
    }
  }

  return;
}
