//
//  swordx2
//  by Giacomo Menchi
//

#ifndef swordx2_h
#define swordx2_h

#include <dirent.h>
#include <getopt.h>

//The longoptions struct lists the available options, their long form and whether they have arguments or not
static struct option longoptions[] = {
    {"alpha",               no_argument,        NULL,   'a' },
    {"exclude",             required_argument,  NULL,   'e' },
    {"follow",              no_argument,        NULL,   'f' },
    {"help",                no_argument,        NULL,   'h' },
    {"ignore",              required_argument,  NULL,   'i' },
    {"log",                 required_argument,  NULL,   'l' },
    {"min",                 required_argument,  NULL,   'm' },
    {"output",              required_argument,  NULL,   'o' },
    {"recursive",           no_argument,        NULL,   'r' },
    {"sortbyoccurrence",    no_argument,        NULL,   's' },
    {0,                     0,                  0,      0   }
};

//Main methods declaration
int optionsManager(int argc, char *argv[]);
void setUpLocale(void);
void inputManager(char *argv[], int index);
void showHelp(void);
void outputSetup(char currentdir[]);
void visitDirectory(char path[]);
void readDirectory(DIR *directory, char fullpath[200], char path[]);
void visitLink(char fullpath[]);
void occurrencesList(char path[]);
void printLog(clock_t end, clock_t begin, char path[]);
void readFile(FILE *filein, wchar_t words[50], int letter, short toignore, short valid);
void checkAndAddWord(wchar_t words[50], int letter, short toignore);
void printWords(void);
void printByOccurrence(FILE *fileout);
short isAlphaOnly(wchar_t word[]);
void addRegisteredWord(void);

#endif /* swordx2_h */
