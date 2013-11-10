#include <modtoollib/modtool.h>
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

bool run( char* command_line )
{
	return system(command_line) == 0;
}