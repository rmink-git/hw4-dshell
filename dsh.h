#define MAXBUF 256  // max number of characteres allowed on command line

// TODO: Any global variables go below

// TODO: Function declarations go below

char** split(char *str, char *delim);

char* trim(char *str);

void freeArray(char** array);

void modeOne(char** command);

void modeTwo(char** command);

