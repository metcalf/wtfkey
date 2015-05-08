#include "wtfkey.h"

int main (int argc, const char * argv[]) {
    if(argc == 2) {
        frequency = atoi(argv[1]);
    }

    if(frequency < 1) {
        fprintf(stderr, "Usage: %s [average frequency of insert]\n", argv[0]);
    }

    if (configureRunloop()){
        fprintf(stderr, "Erroring during setup");
        exit(1);
    }

    if(loadDictionary()) {
        fprintf(stderr, "Erroring loading dictionary");
        exit(1);
    }

    unsigned int seed;
    if(DEBUG) {
        seed = 1;
    } else {
        seed = time(NULL);
    }
    srand(seed);

    debug_print("Starting runloop.\n");
    CFRunLoopRun();

    free(dictionary);
}

int loadDictionary() {
    FILE *fp;
    long lSize;
    unsigned long bytesRead;

    fp = fopen(DICTIONARY_PATH,"r"); // read mode
    if(fp == NULL)
    {
        debug_print("Error while opening the file %s.\n", DICTIONARY_PATH);
        return 1;
    }

   if(fseek(fp, 0, SEEK_END)) {
       fclose(fp);
       debug_print("Error seeking to end of file\n");
       return 2;
   }

   lSize = ftell(fp);
   if(lSize < 0) {
       fclose(fp);
       debug_print("Error reading file location\n");
       return 2;
   }

   rewind(fp);

   dictionary = malloc(lSize);
   if(!dictionary) {
       fclose(fp);
       debug_print("malloc failure for dictionary of size %lu\n", lSize);
       return 3;
   }

   // Read the dictionary into memory
   bytesRead = fread(dictionary, 1, lSize, fp);
   if((unsigned long)lSize != bytesRead) {
       fclose(fp);
       free(dictionary);
       debug_print("Error reading full dictionary. Got %lu of %lu bytes\n", bytesRead, lSize);
       return 4;
   }

   // Loop over the file to find the boundary between letters
   char needle;
   unsigned long pos = 0;

   debug_print("Boundaries: ");
   letterBoundaries[0] = 0;
   for(int i = 1; i < 26; i++) {
       needle = 'a' + i;
       while(true) {
           if((pos+1) >= (unsigned long)lSize) {
               debug_print("Ran past the end of the dictionary looking for '%c'\n", needle);
               return 5;
           }
           if (dictionary[pos] == '\n') {
               wordCounts[i-1]++;
               if(needle == tolower(dictionary[pos+1])) {
                   debug_print("%c:%lu\t", needle, pos+1);
                   letterBoundaries[i] = pos+1;
                   pos++;
                   break;
               }
           }
           pos++;
       }
   }
   letterBoundaries[26] = lSize;
   debug_print("\n");

   while(pos < (unsigned long)lSize) {
       if(dictionary[pos] == '\n') {
           wordCounts[25]++;
       }
       pos++;
   }

   if(DEBUG) {
       debug_print("Word counts: ");
       for(int i = 0; i < 26; i++) {
           needle = 'a' + i;
           debug_print("%c:%d\t", needle, wordCounts[i]);
       }
   }
   debug_print("\n");

   return fclose(fp);
}

int configureRunloop() {
    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyUp);
    CFMachPortRef eventTap = CGEventTapCreate(
        kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionListenOnly, eventMask, keyCallback, NULL
        );

    // Exit the program if unable to create the event tap.
    if(!eventTap) {
        debug_print("ERROR: Unable to create event tap.\n");
        return 1;
    }

    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(
        kCFAllocatorDefault, eventTap, 0
        );
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);

    CFRunLoopSourceContext emittingContext = { .perform = &emitCallback };
    emittingSource = CFRunLoopSourceCreate(NULL, 0, &emittingContext);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), emittingSource, kCFRunLoopCommonModes);

    return 0;
}

CGEventRef keyCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    // Only register key up events with no modifier at a random interval
    if (emitting || type != kCGEventKeyUp || (flagMask & CGEventGetFlags(event)) || (rand() % frequency) != 0) {
        return NULL;
    }

    CGKeyCode keyCode = (CGKeyCode) CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
    emitting = codeToChar(keyCode);

    // Ignore non-characters
    if(!emitting) {
        return NULL;
    }

    CFRunLoopSourceSignal(emittingSource);
    CFRunLoopWakeUp(CFRunLoopGetCurrent());

    return NULL;
}

void emitCallback(void* info)
{
    if(!emitting){
        debug_print("No character to emit\n");
        return; // How did we get here?
    }

    debug_print("emitting fake key presses\n");

    int charIdx = emitting - 'a';

    unsigned int idx = rand() % wordCounts[charIdx];
    unsigned long pos = letterBoundaries[charIdx];

    debug_print("Seeking to %d '%c' word\n", idx, emitting);

    for(unsigned int i = 0; i < idx; pos++) {
        if(dictionary[pos] == '\n') {
            i++;
            pos++;
        }
    }

    debug_print("Reading word starting at %lu\n", pos);

    int code;
    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    CGEventRef key;
    char ch;
    debug_print("Writing: ");
    while(dictionary[pos] != '\n') {
        ch = tolower(dictionary[pos]);
        code = charToCode(ch);
        if(code == 127){
            debug_print("Invalid character %c", ch);
            exit(1);
        }

        debug_print("%c/%d ", ch, code);

        key = CGEventCreateKeyboardEvent(source, code, true);
        CGEventPost(kCGHIDEventTap, key);
        CFRelease(key);

        pos++;
    }
    debug_print("\n");

    CFRelease(source);

    emitting = 0;
}

char codeToChar(int keyCode) {
    switch (keyCode) {
        case 0:   return 'a';
        case 1:   return 's';
        case 2:   return 'd';
        case 3:   return 'f';
        case 4:   return 'h';
        case 5:   return 'g';
        case 6:   return 'z';
        case 7:   return 'x';
        case 8:   return 'c';
        case 9:   return 'v';
        case 11:  return 'b';
        case 12:  return 'q';
        case 13:  return 'w';
        case 14:  return 'e';
        case 15:  return 'r';
        case 16:  return 'y';
        case 17:  return 't';
        case 31:  return 'o';
        case 32:  return 'u';
        case 34:  return 'i';
        case 35:  return 'p';
        case 37:  return 'l';
        case 38:  return 'j';
        case 40:  return 'k';
        case 45:  return 'n';
        case 46:  return 'm';
    }
    return 0;
}

int charToCode(char keyChar) {
    switch (keyChar) {
        case 'a':   return 0;
        case 's':   return 1;
        case 'd':   return 2;
        case 'f':   return 3;
        case 'h':   return 4;
        case 'g':   return 5;
        case 'z':   return 6;
        case 'x':   return 7;
        case 'c':   return 8;
        case 'v':   return 9;
        case 'b':   return 11;
        case 'q':   return 12;
        case 'w':   return 13;
        case 'e':   return 14;
        case 'r':   return 15;
        case 'y':   return 16;
        case 't':   return 17;
        case 'o':   return 31;
        case 'u':   return 32;
        case 'i':   return 34;
        case 'p':   return 35;
        case 'l':   return 37;
        case 'j':   return 38;
        case 'k':   return 40;
        case 'n':   return 45;
        case 'm':   return 46;
        case '-':   return 27;
    }
    return 127;
}
