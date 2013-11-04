#pragma once

#include <string>
#include <vector>

#include <systemlib/platform.h>
#include <renderlib/texture_enums.h>
#include <systemlib/klei_endian.h>
#include <FreeImage.h>

#include "ResizeMethod.h"

//------------------------------------------------------------------------------------------------

StringVec GetEndianNames();
Endian::Type GetEndian( const std::string& name );

//------------------------------------------------------------------------------------------------

StringVec GetPixelFormatNames();
PixelFormat::Type GetPixelFormat( const std::string& name );

//------------------------------------------------------------------------------------------------

StringVec GetPlatformNames();
ePlatform GetPlatform( const std::string& name );

//------------------------------------------------------------------------------------------------

StringVec GetResizeFilterNames();
FREE_IMAGE_FILTER GetResizeFilter( const std::string& name );

//------------------------------------------------------------------------------------------------

StringVec GetResizeMethodNames();
ResizeMethod::Type GetResizeMethod( const std::string& name );

//------------------------------------------------------------------------------------------------

StringVec GetTextureTypeNames();
TextureType::Type GetTextureType( const std::string& name );
