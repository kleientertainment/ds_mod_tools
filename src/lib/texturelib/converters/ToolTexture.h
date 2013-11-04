#pragma once

#include <renderlib/BaseTexture.h>

class ToolTexture : public BaseTexture
{
public:
	ToolTexture( uint32_t num_mips );
	virtual ~ToolTexture();

	void SetMipDescription( uint8_t mip_level, uint16_t width, uint16_t height, uint16_t pitch, uint32_t data_size, const void* mip_data );
	
	virtual void* ToARGB( uint32_t mip_level ) const = 0;

protected:
	static uint32_t NumMipLevels( PixelFormat::Type pixel_format, const InputImages& input_images );
};
