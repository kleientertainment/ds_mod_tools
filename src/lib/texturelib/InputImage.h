#pragma once

#include <FreeImage.h>

#include <renderlib/texture_enums.h>

struct FIBITMAP;

struct Colour
{
	union
	{
		struct  
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		};

		uint32_t mVal;
	};
};

namespace ConversionFlags
{
	enum Type
	{
		SWIZZLE		= 0x00000001,
	};
}

class InputImage
{
public:
	InputImage( const std::string& filename );
	InputImage( FIBITMAP* dib );
	InputImage( uint32_t w, uint32_t h, uint32_t bpp );

	~InputImage();

	uint32_t Width() const;
	uint32_t Height() const;
	uint32_t BPP() const;
	void* RawData() const;
	
	InputImage* ConvertTo32Bit() const;
	InputImage* ConvertTo24Bit() const;
	InputImage* Clone() const;
	
	void Clear();
	bool Paste( const InputImage* src, uint32_t top, uint32_t left, uint32_t alpha = 255 );

	Colour GetPixel( uint32_t x, uint32_t y ) const;
	void SetPixel( uint32_t x, uint32_t y, const Colour& c );
	
	InputImage* Resize( uint32_t w, uint32_t h, FREE_IMAGE_FILTER filter );

	void Save( const char* filename ) const;

	void FlipVertical() const;
	void PremultiplyAlpha();
    void ApplyHardAlpha();
	
private:
	FIBITMAP* mDIB;
};
