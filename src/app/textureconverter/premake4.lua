include( path.join( FOREIGN_SRC_DIR, "build/FreeImage" ) ) 
include( path.join( FOREIGN_SRC_DIR, "build/squish-1.10" ) ) 

ProjectName = "TextureConverter"

project( ProjectName )
	kind( "ConsoleApp" )

	files( { "*.cpp", "*.h" } )

	defines( { "FREEIMAGE_LIB" } )

	includedirs( { "../..", "../TextureLib", FOREIGN_SRC_DIR .. "/build/FreeImage/Source",
					FOREIGN_SRC_DIR .. "/build/tclap-1.2.1/include" } )

	links( { "TextureLib", "renderlib", "systemlib", "util", "ziplib", "FreeImage", "squish"  } )

	Configuration.ConfigureProject( ProjectName )

	configuration( "Release" )
		targetsuffix( "" )

	configuration( "windows" )
		targetdir( path.join( ROOT_DIR, "tools/bin" ) )

	configuration( "linux" )
		includedirs( { "../TextureConverter" } )
		links( { "pthread"  } )
