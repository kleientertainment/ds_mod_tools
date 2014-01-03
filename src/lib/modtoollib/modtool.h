#ifndef MODTOOLLIB_MODTOOL_H
#define MODTOOLLIB_MODTOOL_H

#include <stdio.h>
#define MAX_PATH_LEN 32768

#include <modtools_compat.hpp>

//#define MODTOOLS_DEBUG 1

size_t get_file_size(FILE* f);
char* read_file_append_null(FILE* f);
bool run( char* command_line, bool fail_on_error, char const* format, ... );
void clear_log();
void begin_log();
void end_log();
void show_error_log();
void error( char const* format, ... );
void log( char const* format, ... );
void log_and_print( char const* format, ... );
void log_and_fprint( FILE* print_stream, char const* format, ... );
void create_temp_dir();
char* get_temp_dir();
char* get_asset_temp_dir();
void set_asset_name(char const* name);
void get_folder( char const* path, char* folder );
void set_application_folder( char const* application_path );
char const* get_application_folder();
char const* get_python();

#if defined(DEBUG) && defined(__GNUC__)
/*
 * The '##' prefix for macro varargs is a gcc extension, so I'm just enabling it under it, since I don't know if VS supports it.
 */
#	define error(fmt, ...) {fprintf(stderr, __FILE__":%d ", __LINE__); error(fmt, ##__VA_ARGS__);}
#endif

#endif
