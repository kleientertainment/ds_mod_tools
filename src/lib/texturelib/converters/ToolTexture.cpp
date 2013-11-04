#include "pch.h"

#include "ToolTexture.h"
#include "PixelFormatConverter.h"

#include "../InputImage.h"

ToolTexture::ToolTexture( uint32_t num_mips )
{
	mMetaData.mNumMips = num_mips;
	mMipDescriptions = new sMipDescription[ num_mips ];
}

ToolTexture::~ToolTexture()
{
	for( uint32_t i = 0 ; i < NumMips(); ++i )
	{
		const char* data = reinterpret_cast< const char* > ( mMipDescriptions[ i ].mData );
		delete[] data;
	}
}

void ToolTexture::SetMipDescription( uint8_t mip_level, uint16_t width, uint16_t height, uint16_t pitch, uint32_t data_size, const void* mip_data )
{
	sMipDescription &mip = mMipDescriptions[ mip_level ];
	
	mip.mWidth = width;
	mip.mHeight = height;
	mip.mPitch = pitch;
	mip.mData = mip_data;
	mip.mDataSize = data_size;
}

uint32_t ToolTexture::NumMipLevels( PixelFormat::Type pixel_format, const InputImages& input_images )
{
	uint32_t min_w, min_h;
	PixelFormatConverter::MinTextureDimensions( pixel_format, min_w, min_h );

	uint32_t num_levels = 0;
	while( num_levels < input_images.size() && input_images[ num_levels ]->Width() >= min_w && input_images[ num_levels ]->Height() >= min_h )
	{
		++num_levels;
	}

	return num_levels;
}
