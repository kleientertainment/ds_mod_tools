#include "pch.h"

#include "Helper.h"
#include <FreeImage.h>
#include <util/util.h>


//------------------------------------------------------------------------------------------------

struct EndianPair
{
	Endian::Type mEndianness;
	const char* mName;
};

EndianPair EndianMap[] =
{
	{	Endian::Big,				"big"		},
	{	Endian::Little,				"little"	},
};

Endian::Type GetEndian( const std::string& name )
{
	for( uint32_t i = 0; i < ARRAYSIZE( EndianMap ); ++i )
	{
		if( name == EndianMap[ i ].mName )
		{
			return EndianMap[ i ].mEndianness;
		}
	}

	return Endian::Little;
}

StringVec GetEndianNames()
{
	std::vector< std::string > names;

	for( uint32_t i = 0; i < ARRAYSIZE( EndianMap ); ++i )
	{
		names.push_back( EndianMap[ i ].mName );
	}

	return names;
}

//------------------------------------------------------------------------------------------------

struct PixelFormatPair
{
	PixelFormat::Type mFormat;
	const char* mName;
};

PixelFormatPair PixelFormatMap[] =
{
	{	PixelFormat::BC1,		"bc1"		},
	{	PixelFormat::BC2,		"bc2"		},
	{	PixelFormat::BC3,		"bc3"		},
	{	PixelFormat::X8R8G8B8,	"x8r8g8b8"	},
	{	PixelFormat::A8R8G8B8,	"argb"		},
	{	PixelFormat::R8G8B8,	"rgb"		},
	{	PixelFormat::L16,		"l16"		},
	{	PixelFormat::A8,		"a8"		},
	{	PixelFormat::BC1,		"dxt1"		},
	{	PixelFormat::BC2,		"dxt3"		},
	{	PixelFormat::BC3,		"dxt5"		},
	{	PixelFormat::ATITC_RGB,			"atitc"		},
	{	PixelFormat::ATITC_RGBA_EXP,	"atitc_a_e"	},
	{	PixelFormat::ATITC_RGBA_INT,	"atitc_a_i"	},
	{	PixelFormat::PVRTC_RGB_4,		"pvrtc_4"	},
	{	PixelFormat::PVRTC_RGB_2,		"pvrtc_2"	},
	{	PixelFormat::PVRTC_RGBA_4,		"pvrtca_4"	},
	{	PixelFormat::PVRTC_RGBA_2,		"pvrtca_2"	},
};

const uint32_t NumPixelFormats = sizeof( PixelFormatMap ) / sizeof( PixelFormatPair );

PixelFormat::Type GetPixelFormat( const std::string& name )
{
	for( uint32_t i = 0; i < NumPixelFormats; ++i )
	{
		if( name == PixelFormatMap[ i ].mName )
		{
			return PixelFormatMap[ i ].mFormat;
		}
	}

	return PixelFormat::UNKNOWN;
}

StringVec GetPixelFormatNames()
{
	std::vector< std::string > names;

	for( uint32_t i = 0; i < NumPixelFormats; ++i )
	{
		names.push_back( PixelFormatMap[ i ].mName );
	}

	return names;
}


//------------------------------------------------------------------------------------------------

struct PlatformPair
{
	ePlatform mPlatform;
	const char* mName;
};

PlatformPair PlatformMap[] =
{
	{	PLATFORM_OPENGL,			"opengl"	},
	{	PLATFORM_OPENGLES2,			"gles2"		},
	{	PLATFORM_D3D9,				"d3d9"		},
	{	PLATFORM_XBOX360,			"xbox360"	},
	{	PLATFORM_PS3,				"ps3"		},
	{	PLATFORM_PS4,				"ps4"		},
};

ePlatform GetPlatform( const std::string& name )
{
	for( uint32_t i = 0; i < NUM_PLATFORMS; ++i )
	{
		if( name == PlatformMap[ i ].mName )
		{
			return PlatformMap[ i ].mPlatform;
		}
	}

	return NUM_PLATFORMS;
}

StringVec GetPlatformNames()
{
	std::vector< std::string > names;

	for( uint32_t i = 0; i < NUM_PLATFORMS; ++i )
	{
		names.push_back( PlatformMap[ i ].mName );
	}

	return names;
}

//------------------------------------------------------------------------------------------------

struct ResizeFilterPair
{
	FREE_IMAGE_FILTER mFilter;
	const char* mName;
};

ResizeFilterPair ResizeFilterMap[] =
{
	{	FILTER_BOX,			"box"			},
	{	FILTER_BILINEAR,	"bilinear"		},
	{	FILTER_BSPLINE,		"bspline"		},
	{	FILTER_BICUBIC,		"bicubic"		},
	{	FILTER_CATMULLROM,	"catmulllrom"	},
	{	FILTER_LANCZOS3,	"lanczos3"		},
};

const uint32_t NumResizeFilters = sizeof( ResizeFilterMap ) / sizeof( ResizeFilterPair );

FREE_IMAGE_FILTER GetResizeFilter( const std::string& name )
{
	for( uint32_t i = 0; i < NumResizeFilters; ++i )
	{
		if( name == ResizeFilterMap[ i ].mName )
		{
			return ResizeFilterMap[ i ].mFilter;
		}
	}

	return FILTER_BSPLINE;
}

StringVec GetResizeFilterNames()
{
	std::vector< std::string > names;

	for( uint32_t i = 0; i < NumResizeFilters; ++i )
	{
		names.push_back( ResizeFilterMap[ i ].mName );
	}

	return names;
}

//------------------------------------------------------------------------------------------------

struct TextureTypePair
{
	TextureType::Type mTextureTypeness;
	const char* mName;
};

TextureTypePair TextureTypeMap[] =
{
	{	TextureType::D1,				"1d"		},
	{	TextureType::D2,				"2d"		},
	{	TextureType::D3,				"3d"		},
	{	TextureType::CubeMap,			"cubemap"	},
};

TextureType::Type GetTextureType( const std::string& name )
{
	for( uint32_t i = 0; i < ARRAYSIZE( TextureTypeMap ); ++i )
	{
		if( name == TextureTypeMap[ i ].mName )
		{
			return TextureTypeMap[ i ].mTextureTypeness;
		}
	}

	return TextureType::D2;
}

StringVec GetTextureTypeNames()
{
	std::vector< std::string > names;

	for( uint32_t i = 0; i < ARRAYSIZE( TextureTypeMap ); ++i )
	{
		names.push_back( TextureTypeMap[ i ].mName );
	}

	return names;
}


//------------------------------------------------------------------------------------------------

struct ResizeMethodPair
{
	ResizeMethod::Type mResizeMethod;
	const char* mName;
};

ResizeMethodPair ResizeMethodMap[] =
{
	{	ResizeMethod::Stretch,		"stretch"	},
	{	ResizeMethod::Paste,		"paste"	},
};

ResizeMethod::Type GetResizeMethod( const std::string& name )
{
	for( uint32_t i = 0; i < ARRAYSIZE( EndianMap ); ++i )
	{
		if( name == ResizeMethodMap[ i ].mName )
		{
			return ResizeMethodMap[ i ].mResizeMethod;
		}
	}

	return ResizeMethod::Stretch;
}

StringVec GetResizeMethodNames()
{
	std::vector< std::string > names;

	for( uint32_t i = 0; i < ARRAYSIZE( ResizeMethodMap ); ++i )
	{
		names.push_back( ResizeMethodMap[ i ].mName );
	}

	return names;
}
