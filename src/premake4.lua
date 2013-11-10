os_properties = 
{
	windows = { dir = 'win32' },
	macosx = { dir = 'osx' },
	linux = { dir = 'linux' },
}

props = {}

if _OPTIONS.os == nil then
   error('Please specify your target os!')
elseif os_properties[_OPTIONS.os] == nil then
	error('Unsupported os!')
else
	props = os_properties[_OPTIONS.os]
end

apps = {'scml', 'png', 'autocompiler', 'textureconverter'}

solution('mod_tools')
	configurations { "debug", "release" }
	location "../../ds_mod_tools_out/proj"
	flags { "Symbols", "NoRTTI", "NoEditAndContinue", "NoExceptions", "NoPCH" }
	includedirs { "../lib/" }
  	targetdir ( "../../ds_mod_tools_out/"..props.dir.."/mod_tools" )

    configuration { "debug" }
        defines { "DEBUG" }	 
   	configuration { "release" }
        defines { "RELEASE" }
        flags { "Optimize" }	

	for k, app in pairs(apps) do	
	   	project(app)
			kind "ConsoleApp"
			language "C++"   	   
	      	files { "app/"..app.."/**.h", "app/"..app.."/**.cpp" }	 
	end