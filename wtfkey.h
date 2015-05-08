#ifndef __WTFKEY_H_
#define __WTFKEY_H_

#include <stdlib.h>
#include <stdio.h>
#include <ApplicationServices/ApplicationServices.h>

#define NON_ALPHA 127
#define DEBUG 0
#define DICTIONARY_PATH "/usr/share/dict/words"

#define debug_print(...) \
            do { if (DEBUG) fprintf(stderr, ##__VA_ARGS__); } while (0)

int frequency = 0;
volatile char emitting = 0;
const CGEventFlags flagMask = kCGEventFlagMaskAlphaShift | kCGEventFlagMaskShift | kCGEventFlagMaskControl | kCGEventFlagMaskAlternate | kCGEventFlagMaskCommand | kCGEventFlagMaskSecondaryFn;
CFRunLoopSourceRef emittingSource;

char* dictionary;
unsigned long letterBoundaries[27];
unsigned int wordCounts[26];

int configureRunloop();
int loadDictionary();

CGEventRef keyCallback(CGEventTapProxy, CGEventType, CGEventRef, void*);
void emitCallback(void*);

char codeToChar(int);
int charToCode(char);

#endif
