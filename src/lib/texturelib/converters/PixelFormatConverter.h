#pragma once

class ToolTexture;

#include <renderlib/texture_enums.h>
#include <systemlib/platform.h>

namespace PixelFormatConverter
{
	char* Convert( PixelFormat::Type pixel_format, const InputImage& image, uint32_t& data_size, ePlatform platform );
	char* DXTtoARGB( PixelFormat::Type pixel_format, uint32_t width, uint32_t height, const void* data );
	char* PVRTCtoARGB( PixelFormat::Type pixel_format, uint32_t width, uint32_t height, const void* data );
	char* ATITCtoARGB( PixelFormat::Type pixel_format, uint32_t width, uint32_t height, const void* data, const uint32_t data_size );

	uint32_t Pitch( PixelFormat::Type pixel_format, const InputImage& image );
	uint32_t BytesPerBlock( PixelFormat::Type pixel_format );

	void MinTextureDimensions( PixelFormat::Type pixel_format, uint32_t& w, uint32_t& h );
};

