#include <modtoollib/modtool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

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

char appplication_folder[MAX_PATH_LEN];
void get_folder( char const* path, char* folder )
{
    int length = strrchr( path, '\\' ) - path + 1;
    memcpy( folder, path, length );
    folder[length] = 0;
}

void set_application_folder( char const* application_path )
{
    get_folder( application_path, appplication_folder );
}

char const* get_application_folder()
{
    return appplication_folder;
}


FILE* gLog = 0;
char gLogPath[] = "..\\..\\temp\\autocompiler_log.txt";
char gTempFolder[] = "..\\..\\temp";
char gAssetTempFolder[MAX_PATH_LEN];

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
	sprintf(gAssetTempFolder, "%s\\%s\\%s",  get_application_folder(), get_temp_dir(), name);

	char cmd[MAX_PATH_LEN];
	sprintf(cmd, "mkdir %s", gAssetTempFolder);
	system(cmd);
}

void create_temp_dir()
{
	char cmd[MAX_PATH_LEN];
	sprintf(cmd, "mkdir %s", get_temp_dir());
	system(cmd);
}

void begin_log()
{
	gLog = fopen(gLogPath, "a");
}

void end_log()
{
    if(gLog)
    {
		fflush( gLog );
		fclose( gLog );
    }    
}

void clear_log()
{
	FILE* f = fopen(gLogPath, "w");
	if(f)
	{
		fclose(f);
	}
}


void log( char const* format, ... )
{
	if(gLog)
	{
		char message[4096];
		va_list argptr;
		va_start( argptr, format );
		vsprintf( message, format, argptr );
		va_end( argptr );
		fprintf( gLog, message );
	}
}

void error( char const* format, ... )
{
	char message[4096];
	va_list argptr;
	va_start( argptr, format );
	vsprintf( message, format, argptr );
	va_end( argptr );
	printf( message );

	if(gLog)
	{
		fprintf( gLog, message );
		fflush( gLog );
		fclose( gLog );
	}

	exit( -1 );
}

void show_error_log()
{
	end_log();
    system( gLogPath );
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
