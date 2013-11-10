#include <vector>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <tinydir/tinydir.h>

char appplication_folder[1024];
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

bool run( char* command_line )
{
	return system( command_line ) == 0;
}


struct compiler
{
    compiler( char const* compiler_path )
    :   path( compiler_path )
    {
        int ext_begin = strrchr(compiler_path, '\\') - compiler_path + 1;
        int ext_end = strrchr(compiler_path, '.') - compiler_path - ext_begin;
        extension = path.substr(ext_begin, ext_end);
    }

    void compile( char const* asset_path, char const* output_folder )
    {
        char command_line[1024];
        sprintf( command_line, "%s \"%s\" \"%s\"", path.c_str(), asset_path, output_folder );
        run( command_line );    
    }

    std::string extension;
    std::string path;
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
                    char subfolder[2048];
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
    char search_key[1024];
    sprintf( search_key, "%s\\*", folder, extension );

    WIN32_FIND_DATA find_data;
    HANDLE find_handle = FindFirstFile( search_key, &find_data );
    
    if( find_handle != INVALID_HANDLE_VALUE )
    {
        do
        {
            if( strstr( find_data.cFileName, extension ) && strlen( strstr( find_data.cFileName, extension ) ) == strlen( extension ) )
            {
                char file_path[1024];
                sprintf( file_path, "%s\\%s", folder, find_data.cFileName );
                files.push_back( file_path );
            }
            else if( find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            {
                if( find_data.cFileName[0] != '.' )
                {
                    char subfolder[2048];
                    sprintf( subfolder, "%s\\%s", folder, find_data.cFileName );

                    list_files( subfolder, extension, files );
                }
            }
        }while( FindNextFile( find_handle, &find_data ) );
    }
}

compiler_list get_compilers( char const* folder )
{
    file_list paths;
    list_files( folder, "exe", paths );
    compiler_list compilers;
    for( file_list::iterator iter = paths.begin(); iter != paths.end(); ++iter )
    {
        char const* path = (*iter).c_str();
        compilers.push_back( new compiler( path ) );
    }
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

    char path[1024];
    while( !in.eof() )
    {
        in.getline( path, sizeof( path ) );
        asset_folders.push_back( path );
    }
}


int main( int argument_count, char** arguments )
{
    set_application_folder( arguments[0] );

    file_list mod_folders;
    char mods_folder[1024];
    
    sprintf( mods_folder, "%s..\\..\\mods", get_application_folder() );
    list_folders( mods_folder, mod_folders );

    sprintf( mods_folder, "%s..\\..\\dont_starve\\mods", get_application_folder() );
    list_folders( mods_folder, mod_folders );

    char compilers_folder[1024] ;
    sprintf( compilers_folder, "%scompilers", get_application_folder() );

    compiler_list compilers = get_compilers( compilers_folder );
    for( compiler_list::iterator iter = compilers.begin(); iter != compilers.end(); ++iter )
    {
        compiler* c = *iter;
        file_list asset_folders;
        get_asset_folders( *c, asset_folders );
        
        for( file_list::iterator mod_folder_iter = mod_folders.begin(); mod_folder_iter != mod_folders.end(); ++mod_folder_iter )
        {
            for( file_list::iterator asset_folder_iter = asset_folders.begin(); asset_folder_iter != asset_folders.end(); ++asset_folder_iter )
            {
                char asset_folder[1024];
                sprintf( asset_folder, "%s/%s", mod_folder_iter->c_str(), asset_folder_iter->c_str() );
                compile_folder( c, asset_folder, mod_folder_iter->c_str() );
            }
        }
   }

    return 0;
}