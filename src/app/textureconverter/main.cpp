#include "pch.h"

#include "Helper.h"
#include <InputImage.h>

#include <tclap/CmdLine.h>

#include "Converters/TextureOpenGL.h"
#include "Converters/TexturePS4.h"
//#include "Converters/TexturePS3.h"
//#include "Converters/Texture360.h"

#include <util/BinaryBufferIO.h>

#include <string>
#include <vector>
#include <stack>
#include <stdarg.h>

enum ePrintLevel
{
	PRINT_ERROR = 0,
	PRINT_WARNING,
	PRINT_DEBUG,
};


int gVerbosity = PRINT_ERROR;

// TCLAP will call exit internally. I use this function to set breakpoints while debugging
void ExitCallback()
{
}

void VPrintf( int required_verbosity, const char* format, ... )
{
	if( gVerbosity >= required_verbosity )
	{
		const unsigned int MAX_MSG = 1024;
		char formatted[ MAX_MSG ];

		va_list args;
		va_start( args, format );
		int vlen = vsnprintf( formatted, MAX_MSG, format, args );
		va_end(args);

		printf( "%s", formatted );
	}
}

/**
FreeImage error handler
@param fif Format / Plugin responsible for the error 
@param message Error message
*/
void FreeImageErrorHandler( FREE_IMAGE_FORMAT fif, const char *message )
{
	VPrintf( PRINT_ERROR, "\n*** " ); 
	if( fif != FIF_UNKNOWN )
	{
		VPrintf( PRINT_ERROR, "%s Format\n", FreeImage_GetFormatFromFIF(fif) );
	}
	VPrintf( PRINT_ERROR, message );
	VPrintf( PRINT_ERROR, " ***\n" );
}

InputImages LoadImages( const StringVec& input_filenames )
{
	InputImages images;

	for( StringVec::const_iterator i = input_filenames.begin(), e = input_filenames.end(); i != e; ++i )
	{
		const std::string filename = *i;

		images.push_back( new InputImage( filename ) );
		VPrintf( PRINT_DEBUG, "\t%s\n", filename.c_str() );
	}
	return images;
}

uint32_t RoundUpPow2( uint32_t val )
{
	uint32_t pow2 = 1;
	while( pow2 < val )
	{
		if( pow2 & 0x80000000 )
		{
			break;
		}

		pow2 <<= 1;
	}

	return pow2;
}

void ResizeImages( PixelFormat::Type pixel_format, uint32_t target_width, uint32_t target_height, bool make_pow2, ResizeMethod::Type resize_method, FREE_IMAGE_FILTER filter, InputImages& input_images )
{
	if( ( target_width != 0 || target_height != 0 ) && input_images.size() > 1 )
	{
		// You're specifying a target width/height for multiple images, this doesn't make sense
		// as the images specify a mip-chain. You will have to resize the image yourself.
		throw;
	}

	if( target_width == 0 ) target_width = input_images[ 0 ]->Width();
	if( target_height == 0 ) target_height = input_images[ 0 ]->Height();

	for( uint32_t i = 0; i < input_images.size(); ++i )
	{
		InputImage* img = input_images[ i ];
		
		uint32_t w = img->Width();
		uint32_t h = img->Height();

		if( i == 0 )
		{
			w = target_width;
			h = target_height;
		}

		if( make_pow2 )
		{
			w = RoundUpPow2( w );
			h = RoundUpPow2( h );
		}

		uint32_t target_w = w;
		uint32_t target_h = h;
		if( pixel_format == PixelFormat::BC1 || pixel_format == PixelFormat::BC2 || pixel_format == PixelFormat::BC3 )
		{
			if( target_w % 4 != 0 || target_h % 4 != 0 )
			{
				VPrintf( PRINT_WARNING, "DXT textures must have dimensions that are multiples of 4. Mip level %u has dimensions %ux%u.\nRescaling, but may result in blurriness.\n", i, w, h );
				target_w = target_w - ( target_w % 4 );
				target_h = target_h - ( target_h % 4 );
			}
		}

		if( resize_method == ResizeMethod::Stretch )
		{
			if( target_w != img->Width() || target_h != img->Height() )
			{
				InputImage* new_img = img->Resize( w, h, filter );
				input_images[ i ] = new_img;
				delete img;
			}
		}
		else if( resize_method == ResizeMethod::Paste )
		{
			InputImage* new_img = new InputImage( w, h, img->BPP() );
			new_img->Clear();
			new_img->Paste( img, 0, 0 );
		}
	}
}

void GenerateMips( InputImages& input_images, FREE_IMAGE_FILTER filter )
{
	Assert( input_images.size() == 1 );
	InputImage* img = input_images[ 0 ];

	uint32_t w = img->Width();
	uint32_t h = img->Height();

	while( w > 1 || h > 1 )
	{
		if( w > 1 ) w >>= 1;
		if( h > 1 ) h >>= 1;

		InputImage* mip = img->Resize( w, h, filter );
		input_images.push_back( mip );
	}
}

void PremultiplyAlpha( InputImages& input_images )
{
	VPrintf( PRINT_DEBUG, "Premultiplying alpha\n" );

	for( uint32_t i = 0; i < input_images.size(); ++i )
	{
		InputImage* img = input_images[ i ];
		if( img->BPP() == 32 )
		{
			img->PremultiplyAlpha();
		}
	}

	VPrintf( PRINT_DEBUG, "Done\n" );
}

void ApplyHardAlpha( InputImages& input_images )
{
    VPrintf( PRINT_DEBUG, "Applying hard alpha\n" );

    for( uint32_t i = 0; i < input_images.size(); ++i )
    {
        InputImage* img = input_images[ i ];
        if( img->BPP() == 32 )
        {
            img->PremultiplyAlpha();
        }
    }

    VPrintf( PRINT_DEBUG, "Done\n" );
}


int main( int argc, char* argv[] )
{
	atexit( ExitCallback );

	FreeImage_Initialise();
	FreeImage_SetOutputMessage(FreeImageErrorHandler);

	// Wrap everything in a try block.  Do this every time, because exceptions will be thrown for problems. 
	try
	{
		// Define the command line object.
		TCLAP::CmdLine cmd( "Texture Converter", ' ', "1.0", false );

		TCLAP::ValueArg< std::string > input_filenames_arg( "i", "infiles", "Input texture filenames", true, "", "input.png[;input2.png;input3.png]", cmd );
		TCLAP::ValueArg< std::string > output_filename_arg( "o", "outfile", "Input texture filename", true, "", "output.tex", cmd );

		TCLAP::ValueArg< int > resize_width_arg( "w", "width", "Width to resize the primary mipmap to. No resizing occurs if unspecified.", false, 0, "512", cmd );
		TCLAP::ValueArg< int > resize_height_arg( "h", "height", "Height to resize the primary mipmap to. No resizing occurs if unspecified.", false, 0, "64", cmd );
		TCLAP::ValueArg< int > verbosity_arg( "v", "verbosity", "Output verbosity", false, 0, "0", cmd );

		TCLAP::SwitchArg make_pow2( "", "pow2", "Make primary mipmap a power of two", cmd, false );
		TCLAP::SwitchArg swizzle( "s", "swizzle", "Swizzle the texture on platforms that support it. Imposes platform specific alignment & size restrictions.", cmd, false );
		TCLAP::SwitchArg generate_mips( "", "mipmap", "Generate Mip Maps", cmd, false );
		TCLAP::SwitchArg print_info( "", "info", "Print texture info", cmd, false );
		TCLAP::SwitchArg premultiply( "", "premultiply", "Premultiply alpha", cmd, false );
        TCLAP::SwitchArg hard_alpha( "", "hardalpha", "Clamp alpha 0-1", cmd, false );

		StringVec platform_names = GetPlatformNames();
		TCLAP::ValuesConstraint< std::string > allowed_platforms( platform_names );
		TCLAP::ValueArg< std::string > platform_arg( "p", "platform", "Platform to convert the texture for.", true, "", &allowed_platforms, cmd );

		StringVec pixel_format_names = GetPixelFormatNames();
		TCLAP::ValuesConstraint< std::string > allowed_pixel_formats( pixel_format_names );
		TCLAP::ValueArg< std::string > pixel_format_arg( "f", "format", "Pixel format to convert the texture to", true, "", &allowed_pixel_formats, cmd );

		StringVec resize_filter_names = GetResizeFilterNames();
		TCLAP::ValuesConstraint< std::string > allowed_resize_filters( resize_filter_names );
		TCLAP::ValueArg< std::string > resize_filter_arg( "", "filter", "Resize filter to use", false, "lanczos3", &allowed_resize_filters, cmd );

		StringVec endian_names = GetEndianNames();
		TCLAP::ValuesConstraint< std::string > allowed_endians( endian_names );
		TCLAP::ValueArg< std::string > endian_arg( "e", "endian", "Endianness to output", false, "little", &allowed_endians, cmd );

		StringVec texture_type_names = GetTextureTypeNames();
		TCLAP::ValuesConstraint< std::string > allowed_texture_types( texture_type_names );
		TCLAP::ValueArg< std::string > texture_type_arg( "t", "type", "The type of texture to create", false, "2d", &allowed_texture_types, cmd );

		StringVec resize_method_names = GetResizeMethodNames();
		TCLAP::ValuesConstraint< std::string > allowed_resize_method_names( resize_method_names );
		TCLAP::ValueArg< std::string > resize_method_arg( "r", "resize", "The method used to resize a texture", false, "stretch", &allowed_resize_method_names, cmd );
		const ResizeMethod::Type resize_method = GetResizeMethod( resize_method_arg.getValue() );

		//-------------------------------------------------------------------------------------------------------------------------------------------------
		cmd.parse( argc, argv );

		gVerbosity = verbosity_arg.getValue();

		// TODO: It is possible that you will want to specify each mip of an image. If so, you should
		// modify the above parser to input a list of input filenames

		std::string input_filenames_value = input_filenames_arg.getValue();
		size_t semicolon_pos = input_filenames_value.find( ';' );
		
		StringVec input_filenames;
		
		while( semicolon_pos != std::string::npos )
		{
			std::string filename = input_filenames_value.substr( 0, semicolon_pos );
			input_filenames_value = input_filenames_value.substr( semicolon_pos + 1, std::string::npos );

			input_filenames.push_back( filename );

			semicolon_pos = input_filenames_value.find( ';' );
		}

		input_filenames.push_back( input_filenames_value );


		const std::string output_filename = output_filename_arg.getValue();
		const PixelFormat::Type pixel_format = GetPixelFormat( pixel_format_arg.getValue() );
		const ePlatform platform = GetPlatform( platform_arg.getValue() );
		const TextureType::Type texture_type = GetTextureType( texture_type_arg.getValue() );

		Endian::Type endianness = GetEndian( endian_arg.getValue() );

		switch( platform )
		{
		case PLATFORM_PS3:
		case PLATFORM_XBOX360:
			endianness = Endian::Big;
			break;

		default:
			break;
		}

		//-------------------------------------------------------------------------------------------------------------------------------------------------
		VPrintf( PRINT_DEBUG, "Loading...\n" );
		InputImages input_images = LoadImages( input_filenames );
		VPrintf( PRINT_DEBUG, "Done loading\n" );

		// do the crazy texture processing here where you combine/blend/whatever images
		// should also do resizing etc.
		//
		// At the end of this phase the input texture list should just be a list of textures that represent each
		// mip in the chain

		//-------------------------------------------------------------------------------------------------------------------------------------------------
		FREE_IMAGE_FILTER filter = GetResizeFilter( resize_filter_arg.getValue() );

		VPrintf( PRINT_DEBUG, "Resizing/Generating mips... " );
		ResizeImages( pixel_format, resize_width_arg.getValue(), resize_height_arg.getValue(), make_pow2.getValue(), resize_method, filter, input_images );

		if( generate_mips.getValue() && input_images.size() == 1 )
		{
			GenerateMips( input_images, filter );
		}

		VPrintf( PRINT_DEBUG, "done\n" );

		//-------------------------------------------------------------------------------------------------------------------------------------------------

		if( premultiply.getValue() )
		{
			PremultiplyAlpha( input_images );
		}

        if( hard_alpha.getValue() )
        {
            ApplyHardAlpha( input_images );
        }

		//-------------------------------------------------------------------------------------------------------------------------------------------------



		BaseTexture* output_texture = NULL;

		uint32_t conversion_flags = 0;

		conversion_flags |= swizzle.getValue() ? ConversionFlags::SWIZZLE : 0;

		//-------------------------------------------------------------------------------------------------------------------------------------------------
		VPrintf( PRINT_DEBUG, "Converting... " );
		switch( platform )
		{
		case PLATFORM_OPENGL:		output_texture = TextureOpenGL::Convert( texture_type, pixel_format, conversion_flags, input_images );		break;
		case PLATFORM_OPENGLES2:	output_texture = TextureOpenGL::Convert( texture_type, pixel_format, conversion_flags, input_images );		break;
		case PLATFORM_PS4:			output_texture = TexturePS4::Convert( texture_type, pixel_format, conversion_flags, input_images );		break;

		case PLATFORM_PS3:			VPrintf( PRINT_ERROR, "PS3 not currently supported" ); throw;//output_texture = TexturePS3::Convert( pixel_format, conversion_flags, input_images );		break;
		case PLATFORM_XBOX360:		VPrintf( PRINT_ERROR, "360 not currently supported" ); throw;//output_texture = Texture360::Convert( pixel_format, conversion_flags, input_images );		break;
		}
		VPrintf( PRINT_DEBUG, "done\n" );

		for( InputImages::iterator i = input_images.begin(); i != input_images.end(); ++i )
		{
			InputImage* img = *i;
			delete img;
		}
		input_images.clear();

		//-------------------------------------------------------------------------------------------------------------------------------------------------
		VPrintf( PRINT_DEBUG, "Saving... " );
		std::vector< char > buffer;

		StreamWriter* writer = NULL;


		switch( endianness )
		{
		case Endian::Big:
			writer = new GrowableEndianSwappedBinaryBufferWriter( buffer );
			break;

		case Endian::Little:
			writer = new GrowableBinaryBufferWriter( buffer );
			break;

		default:
			throw;
		}

		output_texture->Serialize( *writer );

		FILE* fp = fopen( output_filename.c_str(), "wb" );
		if( fp != NULL )
		{
			size_t written = fwrite( &buffer[0], buffer.size(), 1, fp );
			Assert( written == 1 );
			fclose( fp );
		}
		else
		{
			std::cerr << "ERROR: Failed to open " << output_filename << " for writing." << std::endl;
			return -2;
		}

		VPrintf( PRINT_DEBUG, "done\n" );

		return 0;
	}
	catch( TCLAP::ArgException &e )  // catch any exceptions
	{
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
	}

	return -1;
}

