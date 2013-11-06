if _OPTIONS.os == nil then
   error('Please specify your target os!')
end

apps = {'scml', 'png', 'autocompiler', 'textureconverter'}

solution('mod_tools')
for k, app in pairs(apps) do	
	   	configurations { "debug", "release" }
	   	location "../gen/proj"
	   	flags { "Symbols", "NoRTTI", "NoEditAndContinue", "NoExceptions", "NoPCH" }
	   	includedirs { "../lib/" }
	   	configuration { "windows" }
	      	targetdir ( "../win32/Dont Starve Mod Tools/compilers" )
	   	configuration { "macosx" }
	      	targetdir ( "../osx/Dont Starve Mod Tools/compilers" )
	   	configuration { "linux" }
	      	targetdir ( "../linux/Dont Starve Mod Tools/compilers" )

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