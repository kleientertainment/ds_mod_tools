#include <modtoollib/modtool.h>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <modtools_compat.hpp>

#ifdef error
#	undef error
#endif

#define DSSTR DIR_SEP_STR_MACRO

#ifndef PYTHONDIR
#	error "The macro PYTHONDIR must be defined."
#endif

#if !defined(va_copy)
#	define va_copy(a, b) ((a) = (b))
#endif

static Compat::Path get_python_root() {
	return Compat::Path(get_application_folder())/"buildtools"/PYTHONDIR/"Python27";
}

/*
 * Changed this to work correctly for files opened in text mode.
 * And returning zero avoids an out of bounds error on indexing the
 * pointer returned by read_file_append_null(), as well as allowing the
 * use of size_t.
 */
size_t get_file_size(FILE* f)
{
	if(!f)
	{
		return 0;
	}
	/*
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	return size;
	*/
	int fd = fileno(f);
	struct stat buf;
	if(fd < 0 || fstat(fd, &buf) != 0) {
		return 0;
	}
	return buf.st_size;
}

char* read_file_append_null(FILE* f)
{
	size_t size = get_file_size(f);
	char* buffer = new char[size + 1];
	if(size > 0) fread(buffer, 1, size, f);
	buffer[size] = 0;
	return buffer;
}

void get_folder( char const* path, char* folder )
{
	const char * last_dirsep = strrchr(path, Compat::Path::SEPARATOR);
	if(last_dirsep == NULL) {
		strcpy(folder, ".");
		return;
	}
    int length = strrchr( path, Compat::Path::SEPARATOR ) - path;
    memcpy( folder, path, length );
    folder[length] = 0;
}

static void initialize_compatibility_layer() {
	static bool initialized = false;
	if(initialized) return;
	initialized = true;
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


static char appplication_folder[MAX_PATH_LEN] = "";
static FILE* gLog = NULL;
static char assetName[1024] = "";

// Suffix of the temporary directory.
#define TEMP_FOLDER_SUFFIX ".."DSSTR".."DSSTR"temp"

// Name of the log.
#define LOG_NAME "autocompiler_log.txt"

void set_application_folder( char const* application_path )
{
    get_folder( application_path, appplication_folder );
	initialize_compatibility_layer();
}

char const* get_application_folder()
{
	if(appplication_folder[0] == '\0') {
		error("ERROR: the application folder has not been set before calling get_application_folder().");
	}
    return appplication_folder;
}



char* get_temp_dir()
{
	static char gTempFolder[MAX_PATH_LEN] = "";

	if(gTempFolder[0] == '\0') {
#ifdef IS_UNIX
		const char * env_path = getenv("MODTOOLS_TEMP_DIR");
		if(env_path) {
			if(strlen(env_path) >= sizeof(gTempFolder)) {
				error("ERROR: buffer overflow on setting temp directory from environment.");
			}
			strcpy(gTempFolder, env_path);
		}
		if(gTempFolder[0] == '\0') {
#endif
			strcpy(gTempFolder, get_application_folder());
			strcat(gTempFolder, DSSTR TEMP_FOLDER_SUFFIX);
#ifdef IS_UNIX
		}
#endif

		if(!Compat::Path(gTempFolder).mkdir()) {
			error("ERROR: Failed to create directory %s.", gTempFolder);
		}
	}
	return gTempFolder;
}

char* get_asset_temp_dir()
{
	static char gAssetTempFolder[MAX_PATH_LEN] = "";
	static bool created = false;
	if(!created) {
		if(assetName[0] == '\0') {
			error("ERROR: the asset name has not been specified before get_asset_temp_dir() was called.");
		}

		created = true;

		sprintf(gAssetTempFolder, "%s"DSSTR"%s",  get_temp_dir(), assetName);

		if(!Compat::Path(gAssetTempFolder).mkdir()) {
			error("ERROR: Failed to create directory %s.", gAssetTempFolder);
		}
	}
	return gAssetTempFolder;
}

void set_asset_name(char const* name)
{
	if(strlen(name) >= sizeof(assetName)) {
		error("ERROR: buffer overflow on setting the asset name.");
	}
	strcpy(assetName, name);
}

void create_temp_dir() {
	(void)get_temp_dir();
}

static const char * getLogPath() {
	static char gLogPath[MAX_PATH_LEN] = "";

	if(gLogPath[0] == '\0') {
#ifdef IS_UNIX
		const char * env_path = getenv("MODTOOLS_LOG");
		if(env_path) {
			if(strlen(env_path) >= sizeof(gLogPath)) {
				error("ERROR: buffer overflow on setting log path from environment.");
			}
			strcpy(gLogPath, env_path);
		}
		if(gLogPath[0] == '\0') {
#endif
			strcpy(gLogPath, get_temp_dir());
			strcat(gLogPath, DSSTR LOG_NAME);
#ifdef IS_UNIX
		}
#endif

	}

	return gLogPath;
}

FILE* open_log(const char * mode) {
	end_log();
	return fopen(getLogPath(), mode);
}

void begin_log()
{
	static bool registered_exit = false;
	
	gLog = open_log("a");

	if(gLog == NULL) {
		error("ERROR: Failed to open log file %s.");
	}

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
	bool was_logging = (gLog != NULL);
	end_log();
	FILE* f = open_log("w");
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
		vfprintf( gLog, format, ap );
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

	log_and_fprint(stderr, "\n");

	end_log();

	exit( -1 );
}

void show_error_log()
{
	end_log();
#ifdef IS_WINDOWS
    system( getLogPath() );
#else
	FILE* f = open_log("r");
	if(f != NULL) {
		fprintf(stderr, "\nLog:\n");

		int c;
		while((c = getc(f)) != EOF) {
			putc(c, stderr);
		}
		putc('\n', stderr);

		fclose(f);
	}
	end_log();
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

bool extract_arg(int& argc, char** argv, const char* name) {
	char opt[128];
	const size_t namelen = strlen(name);
	if(namelen >= sizeof(opt)) {
		error("ERROR: buffer overflow on option extracting.");
	}

	opt[0] = '-';
	if(namelen > 1) {
		opt[1] = '-';
		opt[2] = '\0';
	}
	else {
		opt[1] = '\0';
	}
	strcat(opt, name);

	int howmany = 0;
	for(int i = 1; i < argc;) {
		if(strcmp(argv[i], opt) == 0) {
			for(int j = i + 1; j < argc; j++) {
				argv[j - 1] = argv[j];
			}
			howmany++;
			argc--;
		}
		else {
			i++;
		}
	}
	return howmany > 0;
}
