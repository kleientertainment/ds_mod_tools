#include <vector>
#include <fstream>
#include <string>
#include <tinydir/tinydir.h>
#include <modtoollib/modtool.h>
#include <modtools_compat.hpp>

using namespace Compat;

struct compiler
{
    compiler( Path const& compiler_path )
    :   path( compiler_path )
    {
		log("compiler registered: %s\n", compiler_path.c_str());
        extension = path.basename();
    }

    void compile( char const* asset_path, char const* output_folder )
    {
        char command_line[1024];
#		if defined(IS_WINDOWS)
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
    Path path;
	//Path input_path;
};

typedef std::vector< Path > file_list;
typedef std::vector< compiler* > compiler_list;

void list_folders( Path const& path, file_list& folders )
{
	tinydir_dir dir;
	if (tinydir_open(&dir, path.c_str()) != -1)
	{
		while (dir.has_next)
		{
			tinydir_file file;
			if(tinydir_readfile(&dir, &file) != -1)
			{
				if(file.is_dir && file.name[0] != '.')
				{
					folders.push_back( path/file.name );
				}
			}
			tinydir_next(&dir);
		}
	}
}

void list_files( Path const& folder, char const* extension, file_list& files )
{
	int extension_length = strlen(extension);
	tinydir_dir dir;
	if (tinydir_open(&dir, folder.c_str()) != -1)
	{
		while (dir.has_next)
		{
			tinydir_file file;
			if(tinydir_readfile(&dir, &file) != -1)
			{
				printf("testing file %s\n", file.name);
				if(file.is_dir)
				{
					if(file.name[0] != '.')
					{
						list_files(folder/file.name, extension, files);
					}
				}
				else
				{
					int name_length = strlen(file.name);
					if(name_length > extension_length && strcmp(file.name + name_length - extension_length, extension) == 0)
					{
						files.push_back(folder/file.name);
					}
				}
			}
			tinydir_next(&dir);
		}
	}
	else printf("FAIL at %s\n", folder.c_str());
}

compiler_list get_compilers( Path const& folder )
{
	Path compiler_list_path = folder/"compilers.txt";
	FILE* in = fopen(compiler_list_path.c_str(), "rb");
	if(!in)
	{
		error("ERROR: Could not open '%s'.", compiler_list_path.c_str());
	}

    compiler_list compilers;
	char* buffer = read_file_append_null(in);
	buffer = strtok(buffer, "\r\n");
	do
	{
		char* compiler_name = buffer;
		if(compiler_name[0] != '\0' && compiler_name[0] != '#') {
			compilers.push_back( new compiler( folder/compiler_name ) );
		}
	}while(buffer = strtok(NULL, "\r\n"));

	return compilers;
}

void compile_folder( compiler* c, Path const& asset_folder, Path const& mod_folder )
{
	printf("compiling folder: %s For ext: %s\n", asset_folder.c_str(), c->extension.c_str());
    file_list files;
    list_files( asset_folder, c->extension.c_str(), files );
    for( file_list::iterator file_iter = files.begin(); file_iter != files.end(); ++file_iter )
    {
		printf("file: %s\n", file_iter->c_str());
        c->compile( file_iter->c_str(), mod_folder.c_str() );
    }    
}

void get_asset_folders( compiler& c, file_list& asset_folders )
{
    Path asset_list_path = static_cast<const std::string&>(c.path) + std::string(".txt");
    std::ifstream in( asset_list_path.c_str() );
	if(!in) {
		error("Unable to open %s for reading.", asset_list_path.c_str());
	}

    char path[MAX_PATH_LEN];
    while( !in.eof() )
    {
        in.getline( path, sizeof( path ) );
		const size_t pathlen = strlen(path);
		if(path[pathlen - 1] == '\r') {
			path[pathlen - 1] = '\0';
		}
        asset_folders.push_back( path );
    }
}


int main( int argument_count, char** arguments )
{
	create_temp_dir();
	clear_log();
	begin_log();


    set_application_folder( arguments[0] );
	Path app_folder = get_application_folder();

    file_list mod_folders;
	Path mods_folder = app_folder/".."/".."/"mods";
    list_folders( mods_folder, mod_folders );

	mods_folder = app_folder/".."/".."/"dont_starve"/"mods";
    list_folders( mods_folder, mod_folders );



    compiler_list compilers = get_compilers( get_application_folder() );
    for( compiler_list::iterator iter = compilers.begin(); iter != compilers.end(); ++iter )
    {
		printf("compiler loop: %s\n", (*iter)->path.c_str());
        compiler* c = *iter;
        file_list asset_folders;

        get_asset_folders( *c, asset_folders );
        
        for( file_list::iterator mod_folder_iter = mod_folders.begin(); mod_folder_iter != mod_folders.end(); ++mod_folder_iter )
        {
			printf("mod folder loop: %s\n", mod_folder_iter->c_str());
            for( file_list::iterator asset_folder_iter = asset_folders.begin(); asset_folder_iter != asset_folders.end(); ++asset_folder_iter )
            {
				printf("asset loop: %s\n", asset_folder_iter->c_str());
				compile_folder( c, (*mod_folder_iter)/(*asset_folder_iter), *mod_folder_iter );
            }
        }
   }

	end_log();

    return 0;
}
