#include <vector>
#include <fstream>
#include <string>
#include <tinydir/tinydir.h>
#include <modtoollib/modtool.h>

char appplication_folder[MAX_PATH_LEN];
void set_application_folder( char const* application_path )
{
    int length = strrchr( application_path, '\\' ) - application_path + 1;
    memcpy( appplication_folder, application_path, length );
    appplication_folder[length] = 0;
}

char const* get_application_folder()
{
    return appplication_folder;
}

struct compiler
{
    compiler( char const* compiler_path )
    :   path( compiler_path )
    {
		log("compiler registered: %s\n", compiler_path);
        extension = strrchr(compiler_path, '\\') + 1;
    }

    void compile( char const* asset_path, char const* output_folder )
    {
        char command_line[1024];
#		if defined(WIN32)
			sprintf( command_line, "%s.exe \"%s\" \"%s\"", path.c_str(), asset_path, output_folder );
#		else
			sprintf( command_line, "%s \"%s\" \"%s\"", path.c_str(), asset_path, output_folder );
#		endif
        if(!run( command_line, false, "Compiling %s.", asset_path ))
		{
			show_error_log();
		}
    }

    std::string extension;
    std::string path;
	std::string input_path;
};

typedef std::vector< std::string > file_list;
typedef std::vector< compiler* > compiler_list;

void list_folders( char const* path, file_list& folders )
{
	tinydir_dir dir;
	if (tinydir_open(&dir, path) != -1)
	{
		while (dir.has_next)
		{
			tinydir_file file;
			if(tinydir_readfile(&dir, &file) != -1)
			{
				if(file.is_dir && file.name[0] != '.')
				{
                    char subfolder[MAX_PATH_LEN];
                    sprintf( subfolder, "%s\\%s", path, file.name );
                    folders.push_back( subfolder );	
				}
			}
			tinydir_next(&dir);
		}
	}
}

void list_files( char const* folder, char const* extension, file_list& files )
{
	int extension_length = strlen(extension);
	tinydir_dir dir;
	if (tinydir_open(&dir, folder) != -1)
	{
		while (dir.has_next)
		{
			tinydir_file file;
			if(tinydir_readfile(&dir, &file) != -1)
			{
				if(file.is_dir)
				{
					if(file.name[0] != '.')
					{
						char subfolder[2048];
						sprintf( subfolder, "%s\\%s", folder, file.name );
						list_files(subfolder, extension, files);
					}
				}
				else
				{
					int name_length = strlen(file.name);
					if(name_length > extension_length && strcmp(file.name + name_length - extension_length, extension) == 0)
					{
						char file_path[MAX_PATH_LEN];
						sprintf( file_path, "%s\\%s", folder, file.name );
						files.push_back(file_path);
					}
				}
			}
			tinydir_next(&dir);
		}
	}
}

compiler_list get_compilers( char const* folder )
{
	char compiler_list_path[MAX_PATH_LEN];
	sprintf(compiler_list_path, "%s\\compilers.txt", folder);
	FILE* in = fopen(compiler_list_path, "rb");
	if(!in)
	{
		error("ERROR: Could not open '%s'.", compiler_list_path);
	}

    compiler_list compilers;
	char* buffer = read_file_append_null(in);
	buffer = strtok(buffer, "\r\n");
	do
	{
		char* compiler_name = buffer;

		char compiler_path[MAX_PATH_LEN];
		sprintf(compiler_path, "%s\\%s", folder, compiler_name);

		compilers.push_back( new compiler( compiler_path ) );
	}while(buffer = strtok(0, "\r\n"));

	return compilers;
}

void compile_folder( compiler* c, char const* asset_folder, char const* mod_folder )
{
    file_list files;
    list_files( asset_folder, c->extension.c_str(), files );
    for( file_list::iterator file_iter = files.begin(); file_iter != files.end(); ++file_iter )
    {
        c->compile( file_iter->c_str(), mod_folder );
    }    
}

void get_asset_folders( compiler& c, file_list& asset_folders )
{
    std::string asset_list_path = c.path + std::string( ".txt" );
    std::ifstream in = std::ifstream( asset_list_path.c_str() );

    char path[MAX_PATH_LEN];
    while( !in.eof() )
    {
        in.getline( path, sizeof( path ) );
        asset_folders.push_back( path );
    }
}


int main( int argument_count, char** arguments )
{
	create_temp_dir();
	clear_log();
	begin_log();

    set_application_folder( arguments[0] );

    file_list mod_folders;
    char mods_folder[MAX_PATH_LEN];
    
    sprintf( mods_folder, "%s..\\..\\mods", get_application_folder() );
    list_folders( mods_folder, mod_folders );

    sprintf( mods_folder, "%s..\\..\\dont_starve\\mods", get_application_folder() );
    list_folders( mods_folder, mod_folders );

    compiler_list compilers = get_compilers( get_application_folder() );
    for( compiler_list::iterator iter = compilers.begin(); iter != compilers.end(); ++iter )
    {
        compiler* c = *iter;
        file_list asset_folders;
        get_asset_folders( *c, asset_folders );
        
        for( file_list::iterator mod_folder_iter = mod_folders.begin(); mod_folder_iter != mod_folders.end(); ++mod_folder_iter )
        {
            for( file_list::iterator asset_folder_iter = asset_folders.begin(); asset_folder_iter != asset_folders.end(); ++asset_folder_iter )
            {
                char asset_folder[MAX_PATH_LEN];
                sprintf( asset_folder, "%s\\%s", mod_folder_iter->c_str(), asset_folder_iter->c_str() );
                compile_folder( c, asset_folder, mod_folder_iter->c_str() );
            }
        }
   }

	end_log();

    return 0;
}