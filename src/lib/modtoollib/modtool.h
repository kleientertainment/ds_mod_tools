#include <stdio.h>
#define MAX_PATH_LEN 2048

int get_file_size(FILE* f);
char* read_file_append_null(FILE* f);
bool run( char* command_line );
void clear_log();
void begin_log();
void end_log();
void error( char const* format, ... );
void log( char const* format, ... );
