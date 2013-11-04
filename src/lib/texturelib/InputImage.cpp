#include "pch.h"

#include "InputImage.h"

InputImage::InputImage( const std::string& filename )
	: mDIB( NULL )
{
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	fif = FreeImage_GetFileType( filename.c_str(), 0 );

	if( fif == FIF_UNKNOWN ) fif = FreeImage_GetFIFFromFilename( filename.c_str() );
	if( fif == FIF_UNKNOWN ) throw;

	if(FreeImage_FIFSupportsReading(fif))
	{
		mDIB = FreeImage_Load( fif, filename.c_str() );
		FreeImage_FlipVertical( mDIB );
	}

	if( !mDIB ) throw;
}

InputImage::InputImage( FIBITMAP* dib )
	: mDIB( dib )
{
}


InputImage::InputImage( uint32_t w, uint32_t h, uint32_t bpp )
{
	mDIB = FreeImage_Allocate( w, h, bpp );
}

InputImage::~InputImage()
{
	FreeImage_Unload( mDIB );
}

uint32_t InputImage::Width() const
{
	return FreeImage_GetWidth( mDIB );
}

uint32_t InputImage::Height() const
{
	return FreeImage_GetHeight( mDIB );
}

uint32_t InputImage::BPP() const
{
	return FreeImage_GetBPP( mDIB );
}

void* InputImage::RawData() const
{
	return FreeImage_GetBits( mDIB );
}

InputImage* InputImage::ConvertTo32Bit() const
{
	FIBITMAP* converted = FreeImage_ConvertTo32Bits( mDIB );
	InputImage* result = new InputImage( converted );

	return result;
}

InputImage* InputImage::ConvertTo24Bit() const
{
	FIBITMAP* converted = FreeImage_ConvertTo24Bits( mDIB );
	InputImage* result = new InputImage( converted );

	return result;
}

InputImage* InputImage::Clone() const
{
	return new InputImage( FreeImage_Clone( mDIB ) );
}

Colour InputImage::GetPixel( uint32_t x, uint32_t y ) const
{
	RGBQUAD rgb_quad;
	FreeImage_GetPixelColor( mDIB, x, y, &rgb_quad );

	Colour result;

	result.r = rgb_quad.rgbRed;
	result.g = rgb_quad.rgbGreen;
	result.b = rgb_quad.rgbBlue;
	result.a = rgb_quad.rgbReserved;

	return result;
}

void InputImage::SetPixel( uint32_t x, uint32_t y, const Colour& c )
{
	RGBQUAD rgb_quad;

	rgb_quad.rgbRed = c.r;
	rgb_quad.rgbGreen = c.g;
	rgb_quad.rgbBlue = c.b;
	rgb_quad.rgbReserved = c.a;

	FreeImage_SetPixelColor( mDIB, x, y, &rgb_quad );
}

InputImage* InputImage::Resize( uint32_t w, uint32_t h, FREE_IMAGE_FILTER filter )
{
	return new InputImage( FreeImage_Rescale( mDIB, w, h, filter ) );
}

void InputImage::Save( const char* filename ) const
{
	FreeImage_Save( FIF_TARGA, mDIB, filename );
}

void InputImage::FlipVertical() const
{
	FreeImage_FlipVertical( mDIB );
}

void InputImage::PremultiplyAlpha()
{
	for( uint32_t y = 0; y < Height(); ++y )
	{
		for( uint32_t x = 0; x < Width(); ++x )
		{
			RGBQUAD rgb_quad;
			FreeImage_GetPixelColor( mDIB, x, y, &rgb_quad );

			float alpha = ( rgb_quad.rgbReserved ) / 255.0f;

			rgb_quad.rgbRed = static_cast< BYTE > ( alpha * rgb_quad.rgbRed + 0.5 );
			rgb_quad.rgbGreen = static_cast< BYTE > ( alpha * rgb_quad.rgbGreen + 0.5 );;
			rgb_quad.rgbBlue = static_cast< BYTE > ( alpha * rgb_quad.rgbBlue + 0.5 );;

			FreeImage_SetPixelColor( mDIB, x, y, &rgb_quad );
		}
	}
}

void InputImage::ApplyHardAlpha()
{
    for( uint32_t y = 0; y < Height(); ++y )
    {
        for( uint32_t x = 0; x < Width(); ++x )
        {
            RGBQUAD rgb_quad;
            FreeImage_GetPixelColor( mDIB, x, y, &rgb_quad );

            float alpha = ( rgb_quad.rgbReserved ) / 255.0f;
            if( alpha > 0.5f )
            {
                rgb_quad.rgbReserved = 255;
            }
            else
            {
                rgb_quad.rgbReserved = 0;
            }

            FreeImage_SetPixelColor( mDIB, x, y, &rgb_quad );
        }
    }
}

void InputImage::Clear()
{
	Colour c;
	c.mVal = 0;

	for( uint32_t y = 0; y < Height(); ++y )
	{
		for( uint32_t x = 0; x < Width(); ++x )
		{
			SetPixel( x, y, c );
		}
	}
}

bool InputImage::Paste( const InputImage* src, uint32_t top, uint32_t left, uint32_t alpha )
{
	return FreeImage_Paste( mDIB, src->mDIB, left, top, alpha ) != 0;
}
