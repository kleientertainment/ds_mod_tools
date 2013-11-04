#include "pch.h"


#ifdef _WIN32
	#include <windows.h>
	#define DLLEXPORT __declspec(dllexport)
#else
	#define WINAPI
	#define DLLEXPORT
#endif


#include "Converters/TextureOpenGL.h"
//#include "Converters/Texture360.h"
//#include "Converters/TexturePS3.h"
#include "Converters/TexturePS4.h"
#include "Converters/ToolTexture.h"

#include <util/BinaryBufferIO.h>
#include <systemlib/debug.h>

extern "C"
{
	DLLEXPORT void* WINAPI LoadTexture( const char* filename )
	{
		ToolTexture* result = NULL;

		FILE* fp = fopen( filename, "rb" );
		if( fp != NULL )
		{
			fseek( fp, 0, SEEK_END );
			uint32_t file_size = ftell( fp );
			fseek( fp, 0, SEEK_SET );

			char* file_data = new char[ file_size ];

			int bytes_read = fread( file_data, 1, file_size, fp );
			if( bytes_read == file_size )
			{
				const ToolTexture* raw_texture = reinterpret_cast< const ToolTexture* > ( file_data );

				BinaryBufferReader* reader = NULL;

				switch( raw_texture->Platform() )
				{
				case PLATFORM_XBOX360:
					BREAK();
					//result = new Texture360();
					//reader = new EndianSwappedBinaryBufferReader( file_size, file_data );
					break;

				case PLATFORM_D3D9:
					BREAK();
					//result = new TexturePC();
					//reader = new BinaryBufferReader( file_size, file_data );
					break;

				case PLATFORM_PS3:
					BREAK();
					//result = new TexturePS3();
					//reader = new EndianSwappedBinaryBufferReader( file_size, file_data );
					break;

				case PLATFORM_PS4:
					result = new TexturePS4();
					reader = new BinaryBufferReader( file_size, file_data );
					break;

				case PLATFORM_OPENGL:
				case PLATFORM_OPENGLES2:
					result = new TextureOpenGL();
					reader = new BinaryBufferReader( file_size, file_data );
					break;
				}

				result->DeserializeHeader( *reader, 0 );
				result->DeserializeData( reader );

				delete reader;
			}

			delete[] file_data;

			fclose( fp );
		}

		return result;
	}

	DLLEXPORT void WINAPI UnloadTexture( void* raw_texture )
	{
		const ToolTexture* texture = reinterpret_cast< ToolTexture* > ( raw_texture );
		delete texture;
	}

	DLLEXPORT int WINAPI Format( void* raw_texture )
	{
		const ToolTexture* texture = reinterpret_cast< ToolTexture* > ( raw_texture );
		return texture->PixelFormat();
	}

	DLLEXPORT int WINAPI NumMips( void* raw_texture )
	{
		const ToolTexture* texture = reinterpret_cast< ToolTexture* > ( raw_texture );
		return texture->NumMips();
	}

	DLLEXPORT void* WINAPI GetMip( void* raw_texture, int mip_level )
	{
		ToolTexture* texture = reinterpret_cast< ToolTexture* > ( raw_texture );
		const ToolTexture::sMipDescription& mip_desc = texture->GetMipDescription( mip_level );
		return const_cast< ToolTexture::sMipDescription* > ( &mip_desc );
	}

	DLLEXPORT int WINAPI GetMipWidth( void* mip )
	{
		const ToolTexture::sMipDescription* mip_desc = reinterpret_cast< const ToolTexture::sMipDescription* > ( mip );
		return mip_desc->mWidth;
	}

	DLLEXPORT int WINAPI GetMipHeight( void* mip )
	{
		const ToolTexture::sMipDescription* mip_desc = reinterpret_cast< const ToolTexture::sMipDescription* > ( mip );
		return mip_desc->mHeight;
	}

	DLLEXPORT void* WINAPI GetARGBData( void* raw_texture, int mip_level )
	{
		const ToolTexture* texture = reinterpret_cast< ToolTexture* > ( raw_texture );
		return texture->ToARGB( mip_level );
	}

	DLLEXPORT void WINAPI FreeRGBAData( void* data )
	{
		delete[] data;
	}
}
