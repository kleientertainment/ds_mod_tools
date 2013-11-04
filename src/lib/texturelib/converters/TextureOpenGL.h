#pragma once

#include "ToolTexture.h"

class TextureOpenGL : public ToolTexture
{
public:
	TextureOpenGL();
	TextureOpenGL( uint32_t num_mips );
	virtual ~TextureOpenGL();

public:
	static TextureOpenGL* Convert( TextureType::Type type, PixelFormat::Type pixel_format, uint32_t conversion_flags, InputImages& input_images );
	
	virtual void* ToARGB( uint32_t mip_level ) const;

	virtual bool DeserializeData( BinaryBufferReader* reader );

private:
	virtual void InternalSerialize( StreamWriter& writer ) const;
};
