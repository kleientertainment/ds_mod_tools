ProjectName = "TextureLib"

project( ProjectName )
	config_list = { "Debug", "Release", "Debug DLL", "Release DLL" }
	configurations( config_list )

	files( { "*.cpp", "*.h", "Converters/*.cpp", "Converters/*.h" } )

	excludes( { "Converters/Texture360.*", "Converters/TexturePS3.*" } )

	libdirs( {	FOREIGN_SRC_DIR .. "/build/adrenosdk/lib/win32/vs2008",
				FOREIGN_SRC_DIR .. "/build/powervrsdk/Windows_x86_32/Static" } )

	links( { "AdrenoTextureConverter", "PVRTexLib", "FreeImage", "squish", "ziplib", "renderlib", "util", "systemlib" } )

	includedirs( {	"../..",
					FOREIGN_SRC_DIR .. "/build/FreeImage/Source",
					FOREIGN_SRC_DIR .. "/build/adrenosdk",
					FOREIGN_SRC_DIR .. "/build/powervrsdk/include",
					FOREIGN_SRC_DIR .. "/build/squish-1.10" } )

	configuration( {} )

	XEDK_DIR = os.getenv( "XEDK" )

	if XEDK_DIR ~= nil then
		includedirs( { path.join( XEDK_DIR, "include" ), path.join( path.join( XEDK_DIR, "include" ), "win32" ) } )
	end

	kind( "StaticLib" )

	configuration( "* DLL" )
		kind( "SharedLib" )

	Configuration.ConfigureProject( ProjectName, true, config_list )

	configuration( "*" )
		if _ACTION == "gmake" then
			pchheader( path.join( os.getcwd(), "pch.h" ) )
			pchsource( path.join( os.getcwd(), "pch.cpp" ) )
		else
			pchheader( "pch.h" )
			pchsource( "pch.cpp" )
		end

