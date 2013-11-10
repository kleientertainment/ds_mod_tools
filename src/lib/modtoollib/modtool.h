#include <stdio.h>
#define MAX_PATH_LEN 32768

int get_file_size(FILE* f);
char* read_file_append_null(FILE* f);
bool run( char* command_line, bool fail_on_error, char const* format, ... );
void clear_log();
void begin_log();
void end_log();
void show_error_log();
void error( char const* format, ... );
void log( char const* format, ... );
void create_temp_dir();
char* get_temp_dir();
