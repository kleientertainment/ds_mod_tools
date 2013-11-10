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
	props.outdir = "..\\..\\ds_mod_tools_out\\"
	props.skuoutdir = props.outdir..props.dir.."\\mod_tools\\"
	props.tooldir = '..\\tools\\bin\\'..props.dir.."\\"
end

apps = {'scml', 'png', 'autocompiler', 'textureconverter'}

solution('mod_tools')
	configurations { "debug", "release" }
	location ( props.outdir.."proj" )
	flags { "Symbols", "NoRTTI", "NoEditAndContinue", "NoExceptions", "NoPCH" }
	includedirs { "../lib/" }
  	targetdir ( props.skuoutdir )

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

local function extract(file, folder)
	cmd = props.tooldir..'7z.exe x '..file..' -o'..folder
	os.execute(cmd)	
end

extract('..\\pkg\\cmn\\mod_tools.zip', props.outdir..props.dir)
extract('..\\pkg\\'..props.dir..'\\mod_tools.zip', props.outdir..props.dir)