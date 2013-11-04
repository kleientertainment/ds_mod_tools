#include "pch.h"

#include "PixelFormatConverter.h"
#include "../InputImage.h"
#include "ToolTexture.h"
#include <systemlib/platform.h>

#include <PVRTextureUtilities.h>
#include <PVRTDecompress.h>
#include <squish.h>
#include <TextureConverter.h>

namespace PixelFormatConverter
{
	const int DXT_BLOCK_WIDTH  = 4;
	const int DXT_BLOCK_HEIGHT = 4;
	const int PIXELS_PER_DXT_BLOCK = DXT_BLOCK_WIDTH * DXT_BLOCK_HEIGHT;
	const int BITS_PER_BYTE = 8;


char* DXTCompress( PixelFormat::Type pixel_format, const InputImage& image, uint32_t& compressed_size )
{
	int flags = 0;

	// See squish.h for additional options

	flags |= squish::kColourMetricPerceptual;

	switch( pixel_format )
	{
	case PixelFormat::BC1:		flags |= squish::kDxt1;		break;
	case PixelFormat::BC2:		flags |= squish::kDxt3;		break;
	case PixelFormat::BC3:		flags |= squish::kDxt5;		break;

	default:
		// unexpected format for the DXT converter
		Assert( false );
		break;
	}

	uint32_t w = image.Width();
	uint32_t h = image.Height();
	compressed_size = squish::GetStorageRequirements( w, h, flags );

	char* compressed_data = new char[ compressed_size ];

	//u8 const* rgba
	squish::CompressImage( reinterpret_cast< squish::u8* > ( image.RawData() ), w, h, compressed_data, flags );

	return compressed_data;
}

void ConvertRGBAtoARGB( uint32_t width, uint32_t height, void* data )
{
	struct RGBA
	{
		char r;
		char g;
		char b;
		char a;
	};

	struct ARGB
	{
		char b;
		char g;
		char r;
		char a;
	};

	const RGBA* src_pixel = reinterpret_cast< RGBA* > ( data );
	ARGB* dest_pixel = reinterpret_cast< ARGB* > ( data );

	for( uint32_t i = 0; i < width * height; ++i, ++src_pixel, ++dest_pixel )
	{
		RGBA t = *src_pixel;
		dest_pixel->r = t.r;
		dest_pixel->g = t.g;
		dest_pixel->b = t.b;
		dest_pixel->a = t.a;
	}
}

char* DXTtoARGB( PixelFormat::Type pixel_format, uint32_t width, uint32_t height, const void* data )
{
	uint32_t format = 0;

	switch( pixel_format )
	{
	case PixelFormat::BC1:		format = squish::kDxt1;		break;
	case PixelFormat::BC2:		format = squish::kDxt3;		break;
	case PixelFormat::BC3:		format = squish::kDxt5;		break;

	default:
		Assert( false );
		break;
	}

	uint32_t decompressed_size = width * height * 4; // 4 bytes per ARGB pixel
	char* decompressed = new char[ decompressed_size ];

	squish::DecompressImage( reinterpret_cast< squish::u8* > ( decompressed ), width, height, data, format );

	// Squish decompresses to RGBA but we need ARGB eventually for display purposes
	ConvertRGBAtoARGB( width, height, decompressed );

	return decompressed;
}

char* ConvertToARGB8( const InputImage& image, uint32_t& data_size, ePlatform platform )
{
	const uint32_t bytes_per_pixel = 4;

	// Does pitch need to be adjusted here?
	data_size = image.Width() * image.Height() * bytes_per_pixel;
	char* data = new char[ data_size ];
	char* pixel = data;

	const InputImage* img = &image;
	bool delete_img = false;

	if( image.BPP() < 32 )
	{
		img = image.ConvertTo32Bit();
		delete_img = true;
	}

	const void* raw_data = img->RawData();

	switch( platform ) 
	{
	case PLATFORM_OPENGL:
	case PLATFORM_OPENGLES2:
		for( uint32_t y = 0; y < image.Height(); ++y )
		{
			for( uint32_t x = 0; x < image.Width(); ++x, pixel += 4 )
			{
				Colour c = image.GetPixel( x, y );

				pixel[ 0 ] = c.r;
				pixel[ 1 ] = c.g;
				pixel[ 2 ] = c.b;
				pixel[ 3 ] = c.a;
			}
		}
		break;

	default:
		for( uint32_t y = 0; y < image.Height(); ++y )
		{
			for( uint32_t x = 0; x < image.Width(); ++x, pixel += 4 )
			{
				Colour c = image.GetPixel( x, y );

				pixel[ 0 ] = c.b;
				pixel[ 1 ] = c.g;
				pixel[ 2 ] = c.r;
				pixel[ 3 ] = c.a;
			}
		}
		break;
	}

	if( delete_img ) delete img;

	return data;
}

char* ConvertToRGB8( const InputImage& image, uint32_t& data_size, ePlatform platform )
{
	const uint32_t bytes_per_pixel = 3;

	// Does pitch need to be adjusted here?
	data_size = image.Width() * image.Height() * bytes_per_pixel;
	char* data = new char[ data_size ];
	char* pixel = data;

	const InputImage* img = &image;
	bool delete_img = false;

	if( image.BPP() != 24 )
	{
		img = image.ConvertTo24Bit();
		delete_img = true;
	}

	const void* raw_data = img->RawData();

	switch( platform ) 
	{
	case PLATFORM_OPENGL:
	case PLATFORM_OPENGLES2:
		for( uint32_t y = 0; y < image.Height(); ++y )
		{
			for( uint32_t x = 0; x < image.Width(); ++x, pixel += bytes_per_pixel )
			{
				Colour c = image.GetPixel( x, y );

				pixel[ 0 ] = c.r;
				pixel[ 1 ] = c.g;
				pixel[ 2 ] = c.b;
			}
		}
		break;

	default:
		for( uint32_t y = 0; y < image.Height(); ++y )
		{
			for( uint32_t x = 0; x < image.Width(); ++x, pixel += bytes_per_pixel )
			{
				Colour c = image.GetPixel( x, y );

				pixel[ 0 ] = c.b;
				pixel[ 1 ] = c.g;
				pixel[ 2 ] = c.r;
			}
		}
		break;
	}

	if( delete_img ) delete img;

	return data;
}


int GetATITCFormat( PixelFormat::Type pixel_format )
{
	int result = 0;
	switch( pixel_format )
	{
	case PixelFormat::ATITC_RGB:
		result = Q_FORMAT_ATITC_RGB;
		break;

	case PixelFormat::ATITC_RGBA_EXP:
		result = Q_FORMAT_ATC_RGBA_EXPLICIT_ALPHA;
		break;

	case PixelFormat::ATITC_RGBA_INT:
		result = Q_FORMAT_ATC_RGBA_INTERPOLATED_ALPHA;
		break;
	}

	return result;
}

char *ConvertToATITC(PixelFormat::Type pixel_format, const InputImage &image, uint32_t &data_size)
{
	TQonvertImage q_src;
	memset(&q_src, 0, sizeof(TQonvertImage));
	TQonvertImage q_dst;
	memset(&q_dst, 0, sizeof(TQonvertImage));

	q_src.nWidth = image.Width();
	q_src.nHeight = image.Height();
	q_src.nFormat = (image.BPP() == 32) ? Q_FORMAT_RGBA_8888 : Q_FORMAT_RGB_888;
	q_src.nDataSize = image.Width() * image.Height() * image.BPP() / 8;
	q_src.pData = reinterpret_cast<unsigned char *>(image.RawData());

	q_dst.nWidth = image.Width();
	q_dst.nHeight = image.Height();
	q_dst.nFormat = GetATITCFormat( pixel_format );

	// first call calculates output data size
	unsigned int ret = Qonvert(&q_src, &q_dst);
	if (ret != Q_SUCCESS || q_dst.nDataSize == 0)
	{
		printf("first Qonvert failed %u", ret);
		throw;
	}

	data_size = q_dst.nDataSize;
	char *result = new char[data_size];
	q_dst.pData = reinterpret_cast<unsigned char *>(result);

	ret = Qonvert(&q_src, &q_dst);
	if (ret != Q_SUCCESS)
	{
		printf("second Qonvert failed %u", ret);
		throw;
	}

	return result;
}

char* ATITCtoARGB( PixelFormat::Type pixel_format, uint32_t width, uint32_t height, const void* data, const uint32_t data_size )
{
	TQonvertImage q_src;
	memset(&q_src, 0, sizeof(TQonvertImage));

	TQonvertImage q_dst;
	memset(&q_dst, 0, sizeof(TQonvertImage));

	q_src.nWidth = width;
	q_src.nHeight = height;
	q_src.nFormat = GetATITCFormat( pixel_format );
	q_src.nDataSize = data_size;
	q_src.pData = const_cast< unsigned char* > ( reinterpret_cast<const unsigned char *>( data ) );

	q_dst.nWidth = width;
	q_dst.nHeight = height;
	q_dst.nFormat = Q_FORMAT_RGBA_8888;

	// first call calculates output data size
	unsigned int ret = Qonvert(&q_src, &q_dst);
	if (ret != Q_SUCCESS || q_dst.nDataSize == 0)
	{
		printf("first Qonvert failed %u", ret);
		throw;
	}

	uint32_t decompressed_size = q_dst.nDataSize;
	char *result = new char[ decompressed_size ];
	q_dst.pData = reinterpret_cast<unsigned char *>(result);

	ret = Qonvert(&q_src, &q_dst);
	if (ret != Q_SUCCESS)
	{
		printf("second Qonvert failed %u", ret);
		throw;
	}

	ConvertRGBAtoARGB( width, height, result );

	return result;
}


using namespace pvrtexture;

EPVRTPixelFormat GetPVRFormat( PixelFormat::Type pixel_format )
{
	EPVRTPixelFormat pvr_format;
	switch( pixel_format )
	{
	case PixelFormat::PVRTC_RGB_4:
		pvr_format = ePVRTPF_PVRTCI_4bpp_RGB;
		break;

	case PixelFormat::PVRTC_RGB_2:
		pvr_format = ePVRTPF_PVRTCI_2bpp_RGB;
		break;

	case PixelFormat::PVRTC_RGBA_4:
		pvr_format = ePVRTPF_PVRTCI_4bpp_RGBA;
		break;

	case PixelFormat::PVRTC_RGBA_2:
		pvr_format = ePVRTPF_PVRTCI_2bpp_RGBA;
		break;

	default:
		throw;
		break;
	}

	return pvr_format;
}


char *ConvertToPVRTC(PixelFormat::Type pixel_format, const InputImage &image, uint32_t &data_size)
{
	const InputImage* working_image = &image;

	bool release_image = image.BPP() == 24;
	if( image.BPP() == 24 )
	{
		working_image = image.ConvertTo32Bit();
	}

	CPVRTextureHeader header(PVRStandard8PixelType.PixelTypeID, working_image->Height(), working_image->Width() );
	CPVRTexture tex(header, working_image->RawData());

	if( release_image )
	{
		delete working_image;
	}

	EPVRTPixelFormat pvr_format = GetPVRFormat( pixel_format );
	bool ret = Transcode(tex, pvr_format, ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB);
	if (!ret)
	{
		throw;
	}

	data_size = tex.getDataSize();
	char *result = new char[data_size];
	memcpy(result, tex.getDataPtr(), data_size);


	return result;
}

char* PVRTCtoARGB( PixelFormat::Type pixel_format, uint32_t width, uint32_t height, const void* data )
{
	int pvr2_bit = 0;

	switch( pixel_format )
	{
	case PixelFormat::PVRTC_RGB_4:
	case PixelFormat::PVRTC_RGBA_4:
		pvr2_bit = 0;
		break;

	case PixelFormat::PVRTC_RGB_2:
	case PixelFormat::PVRTC_RGBA_2:
		pvr2_bit = 1;
		break;

	default:
		throw;
		break;
	}

	char* result = new char[ width * height * 4 ]; // decompressed to RGBA
	PVRTDecompressPVRTC( data, pvr2_bit, width, height, reinterpret_cast< uint8_t* > ( result ) );

	ConvertRGBAtoARGB( width, height, result );

	return result;
}

char* Convert( PixelFormat::Type pixel_format, const InputImage& image, uint32_t& data_size, ePlatform platform )
{
	char* result = NULL;

	const InputImage* working_img = &image;
	bool delete_img = false;

	switch( pixel_format )
	{
	case PixelFormat::BC1:
	case PixelFormat::BC2:
	case PixelFormat::BC3:
		if( image.BPP() != 32 )
		{
			working_img = image.ConvertTo32Bit();
			delete_img = true;
		}

		result = DXTCompress( pixel_format, *working_img, data_size );
		break;

	case PixelFormat::A8R8G8B8:
		if( image.BPP() != 32 )
		{
			working_img = image.ConvertTo32Bit();
			delete_img = true;
		}

		result = ConvertToARGB8( *working_img, data_size, platform );
		break;

	case PixelFormat::R8G8B8:
		if( image.BPP() != 24 )
		{
			working_img = image.ConvertTo24Bit();
			delete_img = true;
		}

		result = ConvertToRGB8( *working_img, data_size, platform );
		break;

	case PixelFormat::ATITC_RGB:
	case PixelFormat::ATITC_RGBA_EXP:
	case PixelFormat::ATITC_RGBA_INT:
		result = ConvertToATITC( pixel_format, image, data_size );
		break;

	case PixelFormat::PVRTC_RGB_4:
	case PixelFormat::PVRTC_RGB_2:
	case PixelFormat::PVRTC_RGBA_4:
	case PixelFormat::PVRTC_RGBA_2:
		result = ConvertToPVRTC( pixel_format, image, data_size );
		break;

	default:
		throw;
		break;
	}

	if( delete_img )
	{
		delete working_img;
	}

	return result;
}

uint32_t BytesPerBlock( PixelFormat::Type pixel_format )
{
	uint32_t bytes_per_block = 0;

	switch( pixel_format )
	{
	case PixelFormat::BC1:
		{
			const int num_bits_per_dxt1_block = 4;
			bytes_per_block = PIXELS_PER_DXT_BLOCK * num_bits_per_dxt1_block / BITS_PER_BYTE;
		}
		break;

	case PixelFormat::BC2:
	case PixelFormat::BC3:
		{
			const int num_bits_per_dxt5_block = 8;
			bytes_per_block = PIXELS_PER_DXT_BLOCK * num_bits_per_dxt5_block / BITS_PER_BYTE;
		}
		break;

	case PixelFormat::A8R8G8B8:
		bytes_per_block = 4;
		break;

	case PixelFormat::R8G8B8:
		bytes_per_block = 3;
		break;

	case PixelFormat::ATITC_RGB:
	case PixelFormat::ATITC_RGBA_EXP:
	case PixelFormat::ATITC_RGBA_INT:
		// FIXME: probably not right
		// what does this thing even do?
		bytes_per_block = 1;
		break;

	case PixelFormat::PVRTC_RGB_4:
	case PixelFormat::PVRTC_RGB_2:
	case PixelFormat::PVRTC_RGBA_4:
	case PixelFormat::PVRTC_RGBA_2:
		// FIXME: probably not right
		bytes_per_block = 1;
		break;

	default:
		throw;
	}

	return bytes_per_block;
}

uint32_t Pitch( PixelFormat::Type pixel_format, const InputImage& image )
{
	uint32_t num_bytes_per_block = BytesPerBlock( pixel_format );

	uint32_t pitch = 0;

	switch( pixel_format )
	{
	case PixelFormat::BC1:
	case PixelFormat::BC2:
	case PixelFormat::BC3:
		{
			int num_dxt_blocks_per_row = ( image.Width() + 3 ) / DXT_BLOCK_WIDTH;
			pitch = num_dxt_blocks_per_row * num_bytes_per_block;
		}
		break;

	case PixelFormat::A8R8G8B8:
		pitch = image.Width() * 4;
		break;

	case PixelFormat::R8G8B8:
		pitch = image.Width() * 3;
		break;

	case PixelFormat::ATITC_RGB:
	case PixelFormat::ATITC_RGBA_EXP:
	case PixelFormat::ATITC_RGBA_INT:
		// FIXME: probably not right
		// what does this thing even do?
		pitch = image.Width() / 8;
		break;

	case PixelFormat::PVRTC_RGB_4:
	case PixelFormat::PVRTC_RGBA_4:
		// FIXME: probably not right
		pitch = image.Width() / 8;
		break;

	case PixelFormat::PVRTC_RGB_2:
	case PixelFormat::PVRTC_RGBA_2:
		// FIXME: probably not right
		pitch = image.Width() / 16;
		break;

	default:
		throw;
	}

	return pitch;
}

void MinTextureDimensions( PixelFormat::Type pixel_format, uint32_t& w, uint32_t& h )
{
	switch( pixel_format )
	{
	case PixelFormat::BC1:
	case PixelFormat::BC2:
	case PixelFormat::BC3:
		// OpenGL actually requires that you have a complete mip chain but you must specify whatever stride/block size and pad
		w = 1;
		h = 1;
		break;

	case PixelFormat::A8R8G8B8:
	case PixelFormat::R8G8B8:
		w = 1;
		h = 1;
		break;

	case PixelFormat::ATITC_RGB:
	case PixelFormat::ATITC_RGBA_EXP:
	case PixelFormat::ATITC_RGBA_INT:
		w = 1;
		h = 1;
		break;

	case PixelFormat::PVRTC_RGB_4:
	case PixelFormat::PVRTC_RGB_2:
	case PixelFormat::PVRTC_RGBA_4:
	case PixelFormat::PVRTC_RGBA_2:
		w = 1;
		h = 1;
		break;

	default:
		throw;
	}
}

} // namespace PixelFormatConverter
