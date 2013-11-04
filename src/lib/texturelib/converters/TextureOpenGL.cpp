#include "pch.h"

#include "TextureOpenGL.h"
#include "PixelFormatConverter.h"

#include "../InputImage.h"
#include "ToolTexture.h"

#include <util/BinaryBufferIO.h>
#include <systemlib/platform.h>

#include <cstring>

TextureOpenGL::TextureOpenGL()
: ToolTexture( 0 )
{
	mMetaData.mPlatform = PLATFORM_OPENGL;
}

TextureOpenGL::TextureOpenGL( uint32_t num_mips )
: ToolTexture( num_mips )
{
	mMetaData.mPlatform = PLATFORM_OPENGL;
}

TextureOpenGL::~TextureOpenGL()
{
}

TextureOpenGL* TextureOpenGL::Convert( TextureType::Type type, PixelFormat::Type pixel_format, uint32_t conversion_flags, InputImages& input_images )
{
	const uint32_t num_levels = NumMipLevels( pixel_format, input_images );
	TextureOpenGL* texture = new TextureOpenGL( num_levels );
	
	texture->mMetaData.mPixelFormat = pixel_format;
	texture->mMetaData.mTextureType = type;

	for( uint32_t i = 0; i < num_levels; ++i )
	{
		InputImage& image = *input_images[ i ];
		image.FlipVertical();
		
		uint32_t converted_size;

		const void* converted_pixel_data = PixelFormatConverter::Convert( pixel_format, image, converted_size, PLATFORM_OPENGL );
		if( converted_pixel_data == NULL ) throw;

		const uint32_t pitch = PixelFormatConverter::Pitch( pixel_format, image );

		texture->SetMipDescription( i, image.Width(), image.Height(), pitch, converted_size, converted_pixel_data );
	}

	return texture;
}

void TextureOpenGL::InternalSerialize( StreamWriter& writer ) const
{
	for( uint32_t i = 0; i < NumMips(); ++i )
	{
		const sMipDescription& mip = mMipDescriptions[ i ];
		writer.WriteBytes( mip.mDataSize, mip.mData );
	}
}

bool TextureOpenGL::DeserializeData( BinaryBufferReader* reader )
{
	for( uint32_t i = 0; i < NumMips(); ++i )
	{
		sMipDescription& mip = mMipDescriptions[ i ];
		char* data = new char[ mip.mDataSize ];
		Assert( data != NULL );
		reader->ReadBytes( mip.mDataSize, data );

		mip.mData = data;
	}

	return true;
}

// You must ALWAYS return a copy of the data or a decompressed version of the data. The external system
// WILL be calling back into the library to free the buffer.
void* TextureOpenGL::ToARGB( uint32_t mip_level ) const
{
	PixelFormat::Type pixel_format = PixelFormat();

	const sMipDescription& mip = mMipDescriptions[ mip_level ];

	char* result = NULL;

	switch( pixel_format )
	{
	case PixelFormat::BC1:
	case PixelFormat::BC2:
	case PixelFormat::BC3:
		result = PixelFormatConverter::DXTtoARGB( pixel_format, mip.mWidth, mip.mHeight, mip.mData );
		break;

	case PixelFormat::A8R8G8B8:
		{
			result = new char[ mip.mDataSize ];
			char* dest_pixel = result;
			const char* src_pixel = reinterpret_cast< const char* > ( mip.mData );
			for( uint32_t y = 0; y < mip.mHeight; ++y )
			{
				for( uint32_t x = 0; x < mip.mWidth; ++x, dest_pixel += 4, src_pixel += 4 )
				{
					char a = src_pixel[3];
					char b = src_pixel[2];
					char g = src_pixel[1];
					char r = src_pixel[0];

					dest_pixel[0] = b;
					dest_pixel[1] = g;
					dest_pixel[2] = r;
					dest_pixel[3] = a;
				}
			}
		}
		break;

	case PixelFormat::R8G8B8:
		{
			result = new char[ mip.mWidth * mip.mHeight * 4 ];
			char* dest_pixel = result;
			const char* src_pixel = reinterpret_cast< const char* > ( mip.mData );
			for( uint32_t y = 0; y < mip.mHeight; ++y )
			{
				for( uint32_t x = 0; x < mip.mWidth; ++x, dest_pixel += 4, src_pixel += 3 )
				{
					char b = src_pixel[2];
					char g = src_pixel[1];
					char r = src_pixel[0];

					dest_pixel[0] = b;
					dest_pixel[1] = g;
					dest_pixel[2] = r;
					dest_pixel[3] = 255;
				}
			}
		}
		break;

	case PixelFormat::ATITC_RGB:
	case PixelFormat::ATITC_RGBA_EXP:
	case PixelFormat::ATITC_RGBA_INT:
		result = PixelFormatConverter::ATITCtoARGB( pixel_format, mip.mWidth, mip.mHeight, mip.mData, mip.mDataSize );
		break;

	case PixelFormat::PVRTC_RGB_4:
	case PixelFormat::PVRTC_RGB_2:
	case PixelFormat::PVRTC_RGBA_4:
	case PixelFormat::PVRTC_RGBA_2:
		result = PixelFormatConverter::PVRTCtoARGB( pixel_format, mip.mWidth, mip.mHeight, mip.mData );
		break;
	}

	// OpenGL data is stored bottom left corner pixel first rather than top left corner pixel so we must
	// flip the rows vertically.

	uint32_t row_bytes = mip.mWidth * 4;
	char* temp_row = new char[ row_bytes ];

	for( uint16_t i = 0; i < mip.mHeight / 2; ++i )
	{
		char* top = &result[ i * row_bytes ];
		char* bottom = &result[ ( mip.mHeight - 1 - i ) * row_bytes ];
		memcpy( temp_row, top, row_bytes );
		memcpy( top, bottom, row_bytes );
		memcpy( bottom, temp_row, row_bytes );
	}

	delete[] temp_row;

	return result;
}
