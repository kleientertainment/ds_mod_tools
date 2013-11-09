if _OPTIONS.os == nil then
   error('Please specify your target os!')
end

apps = {'scml', 'png', 'autocompiler', 'textureconverter'}

solution('mod_tools')
for k, app in pairs(apps) do	
	   	configurations { "debug", "release" }
	   	location "../../ds_mod_tools_out/proj"
	   	flags { "Symbols", "NoRTTI", "NoEditAndContinue", "NoExceptions", "NoPCH" }
	   	includedirs { "../lib/" }
	   	configuration { "windows" }
	      	targetdir ( "../../ds_mod_tools_out/win32/mod_tools/compilers" )
	   	configuration { "macosx" }
	      	targetdir ( "../../ds_mod_tools_out/osx/mod_tools/compilers" )
	   	configuration { "linux" }
	      	targetdir ( "../../ds_mod_tools_out/linux/mod_tools/compilers" )

       	configuration { "debug" }
          	defines { "DEBUG" }	 
	   	configuration { "release" }
	        defines { "RELEASE" }
	        flags { "Optimize" }	

	   project(app)
	      kind "ConsoleApp"
	      language "C++"
	      files { "app/"..app.."/**.h", "app/"..app.."/**.cpp" }      
	 
end