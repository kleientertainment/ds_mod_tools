#include <modtoollib/modtool.h>
#include <stdarg.h>
#include <stdlib.h>

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


FILE* gLog = 0;
char gLogPath[] = "..\\..\\temp\\autocompiler_log.txt";
char gTempFolder[] = "..\\..\\temp\\";

char* get_temp_dir()
{
	return gTempFolder;
}

void create_temp_dir()
{
	char cmd[MAX_PATH_LEN];
	sprintf(cmd, "mkdir ", get_temp_dir());
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
    
    system( gLogPath );

	exit( -1 );
}

bool run( char* command_line )
{
	log("running: %s\n", command_line);
	end_log();
	bool result = system(command_line) == 0;
	begin_log();

	return result;
}
