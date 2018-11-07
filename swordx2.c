//
//  swordx2
//  by Giacomo Menchi
//

#include <dirent.h>
#include <getopt.h>
#include <glob.h>
#include <libgen.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include "swordx2.h"
#include "bst.h"

//Variables used for additional options and char validity checking
bool alpha = false;
bool byoccurrence = false;
bool custompath = false;
bool links = false;
bool recursive = false;
char excludedfile[200];
char logfile[200];
char outfile[200];
Node *root = NULL;
Node *ignored = NULL;
int minlength = 0;
int ignoredwords = 0;
int registeredwords = 0;

int main(int argc, char *argv[]) {
    //Sets locale to italian and coding to UTF-8 for accents recognition
    setlocale(LC_ALL, "it_IT.UTF-8");
    memset(excludedfile, 0, 200);
    memset(outfile, 0, 200);
    //Calls method optionsManager which takes input parameters and manages them, returning the index of the first input file/folder
    int index = optionsManager(argc, argv);
    //Should index be less or equal to the number of parameters, one or more input files/folders are given
    if((argc-1) >= index) {
        for(index=index; index<argc; index++) {
            inputManager(argv, index);
        }
    }
    //In case there are no parameters after the options, an error is printed followed by the help
    else {
        if(argc != 1) {
            printf("Error: no input files given or wrong syntax!\n");
            showHelp();
            exit(1);
        }
        showHelp();
        exit(0);
    }
    //After analyzing every file, the program writes the occurrences list and returns 0
    printWords();
    return 0;
}

int optionsManager(int argc, char *argv[]) {
    //getopt_long manages options given by user in both - and -- form, every option found increases the index of the first input parameter
    int optchar, index = 1;
    while((optchar = getopt_long(argc, argv, "ae:fhi:l:m:o:rs", longoptions, NULL)) != -1) {
        switch(optchar) {
                //"a" case: the boolean corresponding to the "alpha" option is put to true
            case 'a': {
                alpha = true;
                index++;
                break;
            }
                //"e" case: the excluded file is saved in a variable and kept for later use
            case 'e': {
                strcpy(excludedfile, argv[index+1]);
                index+=2;
                break;
            }
                //"f" case: the "links" boolean is set to true and will be used later
            case 'f': {
                links = true;
                index++;
                break;
            }
            case 'h': {
                //"h" case: the help is shown and the program stops (if help is requested to be print, it's likely that user doesn't know how to use swordx2)
                showHelp();
                exit(0);
            }
            case 'i': {
                //"i" case: file with ignored words is processed and words are saved in a vector to be excluded from final result
                //First vector element is saved on a pointer or would be lost
                //Opens ignored words file
                FILE *ignoredfile = fopen(argv[index+1], "r");
                if(!ignoredfile) {
                    printf("Error in ignored words file opening!\n");
                    exit(1);
                }
                bool valid = false;
                int letter = 0;
                //Initializes char array in which words will be stored
                wchar_t words[50];
                wmemset(words, 0, 50);
                //Reads characters one by one and checks them
                wchar_t character = getwc(ignoredfile);
                while(!feof(ignoredfile)) {
                    if(iswalnum(character)) {
                        words[letter] = character;
                        character = getwc(ignoredfile);
                        letter++;
                        valid = true;
                    }
                    //As soon as an invalid character is found, the word is added a terminating character (\0) and is copied to the forementioned vector
                    if(!valid) {
                        if(letter != 0) {
                            words[letter] = '\0';
                            ignored = insert(ignored, words);
                        }
                        //Word and letter index are initialized
                        wmemset(words, 0, 50);
                        character = getwc(ignoredfile);
                        letter = 0;
                    }
                    valid = false;
                }
                fclose(ignoredfile);
                index+=2;
                break;
            }
                //"l" case: every file read is written to a log file, with timing and execution details
            case 'l': {
                strcpy(logfile, argv[index+1]);
                FILE *log = fopen(logfile, "w");
                fprintf(log, "SWORDX2 LOG FILE\n\n");
                fclose(log);
                index+=2;
                break;
            }
                //"m" case: the minimum  number of occurrences is stored in a variable and will be used during file writing
            case 'm': {
                minlength = atoi(argv[index+1]);
                index+=2;
                break;
            }
                //"o" case: the output path is saved to the "outfile" variable and the "custompath" variable reminds that was user-specified
            case 'o': {
                strcpy(outfile, argv[index+1]);
                custompath = true;
                index+=2;
                break;
            }
                //"r" case: turns on the recursion
            case 'r': {
                recursive = true;
                index++;
                break;
            }
                //"s" case: words will be rearranged in occurences number order in print method
            case 's': {
                byoccurrence = true;
                index++;
                break;
            }
                //"?" case: if an unknown variable is given, an error message appears and the help is printed
            case '?': {
                showHelp();
                exit(1);
            }
            default: {
                break;
            }
        }
    }
    return index;
}

void inputManager(char *argv[], int index) {
    //Uses glob library to manage inputs and regular expressions (also called wildcards), like * or ?
    glob_t fullpath;
    if(glob(argv[index], 0, NULL, &fullpath) != GLOB_NOMATCH) {
        for(int i = 0; i < fullpath.gl_pathc; i++) {
            struct stat pathstat;
            lstat(fullpath.gl_pathv[i], &pathstat);
            //If path corresponds to a directory, the program to visit it is called
            if(S_ISDIR(pathstat.st_mode)) {
                if(fullpath.gl_pathv[i][strlen(fullpath.gl_pathv[i])-1] != '/') {
                    strcat(fullpath.gl_pathv[i], "/");
                }
                visitDirectory(fullpath.gl_pathv[i]);
            }
            //If path is a regular file, the program calls occurrences list method, otherwise is ignored
            else if(S_ISREG(pathstat.st_mode)) {
                outputSetup(dirname(argv[index]));
                occurrencesList(fullpath.gl_pathv[i]);
            }
        }
    }
    else {
        printf("Error: %s doesn't exist!\n", argv[index]);
        exit(1);
    }
}

void showHelp() {
    //Simply prints help
    printf("\nswordx2 ==> reads from one or a bunch of text files and prints the number of word occurrences to an output\n\n");
    printf("Usage: swordx2 [options] [inputs]\n");
    printf("\nOptions:\n");
    printf("  -a, --alpha\t\t\tonly alphabetical characters are considered\n");
    printf("  -e <file>, --exclude <file>\texclude determined file, only makes sense if used while working in a directory\n");
    printf("  -f, --follow\t\t\tfollow links while working in a directory\n");
    printf("  -h, --help\t\t\tdisplay help message and exit\n");
    printf("  -i <file>, --ignore <file>\tspecify a list of words, one per line, that will be ignored\n");
    printf("  -l <file>, --log <file>\tgenerates a log containing information on file analyzed\n");
    printf("  -m <num>, --min <num>\t\tonly words with minimum length given are considered\n");
    printf("  -o <file>, --output <file>\tgive custom output path\n");
    printf("  -r, --recursive\t\tsearch for occurrences in subdirectories too\n");
    printf("  -s, --sortbyoccurrence\tsort words found by occurrences\n");
    printf("\nInputs:\n");
    printf("  path/file\t\t\tspecify file path\n");
    printf("  path/directory\t\tspecify entire directory\n");
}

void outputSetup(char currentdir[]) {
    //Manages output location choice
    char directory[200];
    memset(directory, 0, 200);
    strcpy(directory, currentdir);
    //If "outfile" is empty, the current directory is set as output path
    if(strlen(outfile) == 0) {
        strcpy(outfile, directory);
        strcat(outfile, "/swordx2.out");
    }
    //If, otherwise, is not empty and not set to custom, the old and the new one are compared and, if different, swordx2 folder is set as output
    else if((strcmp(outfile, strcat(directory, "/swordx2.out")) != 0) && !(custompath)) {
        memset(outfile, 0, 200);
        strcpy(outfile, "swordx2.out");
    }
}

void visitDirectory(char path[]) {
    //If one input is a folder, the directory is scanned searching for files
    DIR *directory = opendir(path);
    if(!directory) {
        printf("Error in directory opening!\n");
        exit(1);
    }
    outputSetup(path);
    char fullpath[200];
    memset(fullpath, 0, 200);
    readDirectory(directory, fullpath, path);
}

void readDirectory(DIR *directory, char fullpath[200], char path[]) {
    while(true) {
        //dirent library is used to manage directory and nested directories (recursive option)
        struct dirent *dir;
        dir = readdir(directory);
        if(!dir) {
            break;
        }
        strcpy(fullpath, path);
        strcat(fullpath, dir->d_name);
        struct stat filestat;
        lstat(fullpath, &filestat);
        //Checks if path is a link and if links are allowed and, in affermative case, it is managed by the program
        if(S_ISLNK(filestat.st_mode) && links) {
            visitLink(fullpath);
        }
        //Checks if path is a folder and if recursivity is requested ("." starting directories are banned because represent current and previous one)
        if(S_ISDIR(filestat.st_mode) && dir->d_name[0] != '.') {
            if(recursive) {
                strcat(fullpath, "/");
                visitDirectory(fullpath);
            }
        }
        //If, last but not least, path represents a regular file not starting with "." (hidden) or ending with "~" (being modified), usual method is called
        else if(dir->d_name[0] != '.' && dir->d_name[strlen(dir->d_name)-1] != '~' && DT_REG == dir->d_type) {
            occurrencesList(fullpath);
        }
    }
}

void visitLink(char fullpath[]) {
    char link[100];
    memset(link, 0, 100);
    readlink(fullpath, link, sizeof(link));
    link[strlen(link)] = '\0';
    //Links can lead to folders, regular files, but also to files that have to be ignored in our statistic
    struct stat linkstat;
    lstat(link, &linkstat);
    if(S_ISDIR(linkstat.st_mode)) {
        visitDirectory(link);
    }
    else if(S_ISREG(linkstat.st_mode)) {
        occurrencesList(link);
    }
    else {
        printf("Error: %s is not regular file or directory\n", link);
        exit(1);
    }
}

void occurrencesList(char path[]) {
    clock_t begin = clock();
    //If path corresponds to an excluded file, the method stops immediately
    if(strcmp(path, excludedfile) == 0) {
        return;
    }
    wchar_t words[50];
    wmemset(words, 0, 50);
    int letter = 0;
    bool toignore = false;
    bool valid = false;
    FILE *filein = fopen(path, "r");
    if(!filein) {
        printf("Error in opening input file %s!\n", path);
        exit(1);
    }
    readFile(filein, words, letter, toignore, valid);
    clock_t end = clock();
    if(strlen(logfile) != 0) {
        printLog(end, begin, path);
    }
}

void printLog(clock_t end, clock_t begin, char path[]) {
    double exectime = (double)(end-begin) / CLOCKS_PER_SEC;
    char timebuffer[20];
    struct tm *sTm;
    time_t now = time(0);
    sTm = localtime(&now);
    strftime(timebuffer, sizeof(timebuffer), "%d-%m-%Y %H:%M:%S", sTm);
    FILE *log = fopen(logfile, "a");
    fprintf(log, "[%s]\t", timebuffer);
    fprintf(log, "Filepath: %s\t", path);
    fprintf(log, "Registered words: %d\t", registeredwords);
    fprintf(log, "Ignored words: %d\t", ignoredwords);
    fprintf(log, "Execution time: %f seconds\n", exectime);
    registeredwords = 0;
    ignoredwords = 0;
    fclose(log);
}

void readFile(FILE *filein, wchar_t words[50], int letter, bool toignore, bool valid) {
    wchar_t character = getwc(filein);
    while(!feof(filein)) {
        //If character is alphanumeric or belongs to the chosen locale, it is chosen as valid
        if(iswalnum(character)) {
            words[letter] = character;
            character = getwc(filein);
            letter++;
            valid = true;
        }
        if(!valid) {
            if(letter != 0) {
                checkAndAddWord(words, letter, toignore);
            }
            wmemset(words, 0, 50);
            character = getwc(filein);
            letter = 0;
            toignore = false;
        }
        valid = false;
    }
    fclose(filein);
}

void checkAndAddWord(wchar_t words[50], int letter, bool toignore) {
    words[letter] = '\0';
    //When "-m" is chosen, the letter is length-checked before adding
    if(letter < minlength) {
        toignore = true;
    }
    //Should "-i" option be active, every word is checked if it has be ignored before being added to the list
    if(ignored && !toignore) {
        Node *start = ignored;
        ignored = search(ignored, words);
        if(ignored) {
            toignore = true;
        }
        ignored = start;
    }
    if(!toignore) {
        root = insert(root, words);
    } else {
        ignoredwords++;
    }
}

void printWords(void) {
    FILE *fileout = NULL;
    fileout = fopen(outfile, "w");
    if(!fileout) {
        printf("Error in output file opening!\n");
        exit(1);
    }
    //Words list can be sorted alphabetically or by occurrences
    if(!byoccurrence) {
        visitInorder(fileout, root, alpha);
    }
    else {
        printByOccurrence(fileout);
    }
    fclose(fileout);
}

void printByOccurrence(FILE *fileout) {
    //The occurrence sorted one must be reordered before writing it to output
    Node* mostfrequent = NULL;
    while(true) {
        findFirstOccurrence(fileout, root, alpha);
        mostfrequent = getmostfrequent();
        if(!mostfrequent) {
            break;
        }
        fprintf(fileout, "%ls %d\n", mostfrequent->occurrence, mostfrequent->wordcounter);
        mostfrequent->wordcounter = -1;
        mostfrequent = NULL;
    }
}

bool isAlphaOnly(wchar_t word[]) {
    //Checks if word contains numbers and, in case, returns false; in any other case the result is true
    int i = 0;
    while(i < wcslen(word)) {
        if(iswdigit(word[i])) {
            return false;
        }
        i++;
    }
    return true;
}

void addRegisteredWord(void) {
    registeredwords++;
}
