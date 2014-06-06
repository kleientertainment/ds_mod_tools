#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <math.h>
#include <modtoollib/modtool.h>
#include <modtools_compat.hpp>

using namespace std;
using namespace Compat;

/*
time_t get_last_modified( char const* path )
{
    struct stat info;
    int result = stat( path, &info );
    if( result == 0 )
    {
        return info.st_mtime;
    }
    else
    {
        return 0;
    }
    
}

bool is_more_recent( char const* path_a, char const* path_b )
{
    time_t a_time = get_last_modified( path_a );
    time_t b_time = get_last_modified( path_b );
    return a_time < b_time;
}

bool exists( char const* path )
{
	struct _stat info;
	int result = _stat( path, &info );
	return result == 0;
}

char const* get_file_name( char const* path )
{
	char const* name = strrchr( path, '/' );
	if( !name )
	{
		return path;		
	}
	return name;
}

void get_output_file_path( char const* input_file_path, char* output_file_path )
{
    strcpy( output_file_path, input_file_path );
    strcpy( strrchr( output_file_path, '.' ), ".xml" );
}
*/

int main( int argument_count, char** arguments )
{
    set_application_folder( arguments[0] );
	set_asset_name( "" );
    begin_log();

	if( argument_count != 3 )
	{
		error( "ERROR: Invalid number of arguments!\n" );
	}

	Path input_file_path = arguments[1];

	if( !input_file_path.exists() )
	{
		error( "ERROR: Could not open '%s'!\n", input_file_path.c_str() );
	}

    Path input_folder = input_file_path.dirname();

	Path output_package_file_path = input_file_path.copy().replaceExtension("xml");

	Path build_tool_path = Path(get_application_folder())/"compiler_scripts"/"image_build.py";

	/*
	 * Existence checks are implicit. See the comment in scml/main.cpp or the
	 * Path class definition for more details.
	 */
	if(
		input_file_path.isOlderThan(output_package_file_path)
		&& Path(arguments[0]).isOlderThan(output_package_file_path)
		&& build_tool_path.isOlderThan(output_package_file_path)
	) {
		log_and_print("%s is up to date.\n", output_package_file_path.c_str());
        return 0;
    }

    // std::vector< char const* > image_paths;

	char command_line[4096];

	/*
    Path output_dir = "data";
    if( input_folder.find("mods") )
    {
        output_dir = ".";
    }
	*/

#if defined(IS_WINDOWS)
    sprintf( command_line, "\"\"%s\" \"%s\" \"%s\"\"", get_python(), build_tool_path.c_str(), input_file_path.c_str());
#else
	sprintf( command_line, "\"%s\" \"%s\" \"%s\"", get_python(), build_tool_path.c_str(), input_file_path.c_str());
#endif
    
    run( command_line, true, "Compiling '%s'", input_file_path.c_str() );

    end_log();

	return 0;
}
