#include <modtoollib/modtool.h>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <modtools_compat.hpp>

#ifdef error
#	undef error
#endif

#define DSSTR DIR_SEP_STR_MACRO

#ifdef IS_WINDOWS
#	define MKDIR "mkdir"
#else
#	define MKDIR "mkdir -p"
#endif

#ifndef PYTHONDIR
#	error "The macro PYTHONDIR must be defined."
#endif

static Compat::Path get_python_root() {
	return Compat::Path(get_application_folder())/"buildtools"/PYTHONDIR/"Python27";
}

int get_file_size(FILE* f)
{
	if(!f)
	{
		return -1;
	}

	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	return size;
}

char* read_file_append_null(FILE* f)
{
	int size = get_file_size(f);
	char* buffer = new char[size + 1];
	fread(buffer, 1, size, f);
	buffer[size] = 0;
	return buffer;
}

static char appplication_folder[MAX_PATH_LEN];
void get_folder( char const* path, char* folder )
{
    int length = strrchr( path, Compat::Path::SEPARATOR ) - path + 1;
    memcpy( folder, path, length );
    folder[length] = 0;
}

static void initialize_compatibility_layer() {
#ifdef IS_UNIX
	/*
	 * Extends the Python path for module searching.
	 *
	 * Works pretty much like the site.py script, but removes its dependency under Unix.
	 * (and allows using a system wide Python installation instead of a custom one)
	 */

	char cstr_absolute_python_root[MAX_PATH_LEN];
	realpath(get_python_root().c_str(), cstr_absolute_python_root);
	Compat::Path absolute_python_root(cstr_absolute_python_root);

	const char * const old_pythonpath = getenv("PYTHONPATH");
	Compat::Path basic_custom_pythonpath = absolute_python_root/"Lib";
	Compat::Path site_custom_pythonpath = basic_custom_pythonpath/"site-packages";
	std::string new_pythonpath = basic_custom_pythonpath + ":" + site_custom_pythonpath;
	if(old_pythonpath != NULL) {
		new_pythonpath += ":";
		new_pythonpath += old_pythonpath;
	}
	setenv("PYTHONPATH", new_pythonpath.c_str(), 1);
#endif
}

void set_application_folder( char const* application_path )
{
    get_folder( application_path, appplication_folder );
	initialize_compatibility_layer();
}

char const* get_application_folder()
{
    return appplication_folder;
}


static FILE* gLog = NULL;
static char gLogPath[] = ".."DSSTR".."DSSTR"temp"DSSTR"autocompiler_log.txt";
static char gTempFolder[] = ".."DSSTR".."DSSTR"temp";
static char gAssetTempFolder[MAX_PATH_LEN];

char* get_temp_dir()
{
	return gTempFolder;
}

char* get_asset_temp_dir()
{
	return gAssetTempFolder;
}

void set_asset_name(char const* name)
{
	sprintf(gAssetTempFolder, "%s"DSSTR"%s"DSSTR"%s",  get_application_folder(), get_temp_dir(), name);

	char cmd[MAX_PATH_LEN];
	sprintf(cmd, MKDIR" %s", gAssetTempFolder);
	system(cmd);
}

void create_temp_dir()
{
	char cmd[MAX_PATH_LEN];
	sprintf(cmd, MKDIR" %s", get_temp_dir());
	system(cmd);
}


void begin_log()
{
	static bool registered_exit = false;

	end_log();
	gLog = fopen(gLogPath, "a");

	if(!registered_exit) {
		registered_exit = (atexit(end_log) == 0);
	}
}

void end_log()
{
    if(gLog)
    {
		fflush( gLog );
		fclose( gLog );
		gLog = NULL;
    }    
}

void clear_log()
{
	bool was_logging = static_cast<bool>(gLog);
	end_log();
	FILE* f = fopen(gLogPath, "w");
	if(f)
	{
		fclose(f);
	}
	if(was_logging) {
		begin_log();
	}
}

static void vlog( char const* format, va_list ap )
{
	if(gLog)
	{
		fprintf( gLog, format, ap );
	}
}

void log( char const* format, ... )
{
	va_list ap;
	va_start( ap, format );
	vlog( format, ap );
	va_end( ap );
}

static void vlog_and_fprint( FILE* print_stream, char const* format, va_list ap )
{
	va_list ap1;
	va_copy( ap1, ap );
	vfprintf( print_stream, format, ap1 );
	va_end( ap1 );

	va_list ap2;
	va_copy( ap2, ap );
	vlog( format, ap2 );
	va_end( ap2 );
}

void log_and_fprint( FILE* print_stream, char const* format, ... )
{
	va_list ap;
	va_start( ap, format );
	vlog_and_fprint( print_stream, format, ap );
	va_end( ap );
}

void log_and_print( char const* format, ... )
{
	va_list ap;
	va_start( ap, format );
	vlog_and_fprint( stdout, format, ap );
	va_end( ap );
}

void error( char const* format, ... )
{
	va_list ap;
	va_start( ap, format );
	vlog_and_fprint( stderr, format, ap );
	va_end( ap );

	// Automatic: see begin_log().
	//end_log();

	exit( -1 );
}

void show_error_log()
{
	end_log();
#ifdef IS_WINDOWS
    system( gLogPath );
#else
	system( (std::string("echo; echo Log:; cat \"") + gLogPath + "\"; echo;").c_str() );
#endif
	exit( -1 );
}

bool run( char* command_line, bool fail_on_error, char const* format, ... )
{
	char message[4096];
	va_list argptr;
	va_start( argptr, format );
	vsprintf( message, format, argptr );
	va_end( argptr );

	log("\n\n>> %s\n\n=================\n\n%s\n\n", message, command_line );
	end_log();
	bool result = system(command_line) == 0;
	begin_log();
	if(!result && fail_on_error)
	{
		error("ERROR: Command failed!");
	}

	return result;
}

static Compat::Path cached_python_path;
char const* get_python() {
	if(cached_python_path.empty()) {
#ifdef IS_WINDOWS
		cached_python_path = get_python_root()/"python.exe";
		if(!cached_python_path.exists()) {
			error("Unable to find python!");
		}
#else
		const char *possibilities[] = {"python2.7", "python2"};
		for(size_t i = 0; i < sizeof(possibilities)/sizeof(possibilities[0]); i++) {
			std::string attempt = std::string() + "which " + possibilities[i] + " &>/dev/null";
			if(system(attempt.c_str()) == 0) {
				cached_python_path = possibilities[i];
				break;
			}
		}
		if(cached_python_path.empty()) {
			error("Unable to find python!");
		}
#endif
	}
	return cached_python_path.c_str();
}
