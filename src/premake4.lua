os_properties = 
{
	windows = { dir = 'win32', pythondir = 'windows' },
	macosx = { dir = 'osx', pythondir = 'osx' },
	linux = { dir = 'linux', pythondir = 'linux' },
}


-------------------------------------------------------------------------

--[[
-- Tries to infer the host OS.
--]]
local function deduceOS()
	--[[
	-- First we distinguish between Windows and *nix by the native directory
	-- separator.
	--]]
	local DIR_SEP = package.config:sub(1,1)
	if DIR_SEP == "\\" then
		return "windows"
	elseif DIR_SEP == "/" then
		-- Unix.
		local kernel_name = (function()
			local uname = io.popen("uname", "r")
			if uname then
				local kern = uname:read("*a"):gsub("^%s+", ""):gsub("%s+$", "")
				uname:close()
				return kern:lower()
			end
		end)()
		if kernel_name == "linux" then
			return "linux"
		elseif kernel_name == "darwin" then
			return "macosx"
		end
	end
end



if _OPTIONS.os == nil then
	_OPTIONS.os = deduceOS()
end
if _OPTIONS.os == nil then
   error('Please specify your target os!')
elseif os_properties[_OPTIONS.os] == nil then
	error('Unsupported os!')
end


-------------------------------------------------------------------------
--[[
-- Utility functions.
--]]

local function targetIsUnix()
	return _OPTIONS.os == "linux" or _OPTIONS.os == "macosx"
end

local function targetIsWindows()
	return _OPTIONS.os == "windows"
end

assert( targetIsUnix() or targetIsWindows() )

local hostIsUnix, hostIsWindows
do
	local host = assert( deduceOS, "Unable to infer the host OS." )

	local function True() return true end
	local function False() return false end

	if host == "windows" then
		hostIsUnix, hostIsWindows = False, True
	else
		hostIsUnix, hostIsWindows = True, False
	end
end

-- Concatenates path components using the native directory separator.
local function catfile(...)
	return table.concat({...}, package.config:sub(1,1))
end

-- Creates a directory, including intermediate ones.
local mkdir = (function()
	--[[
	-- Command formatting string.
	--]]
	local cmd_template
	if hostIsUnix() then
		cmd_template = "mkdir -p %q"
	else
		cmd_template = "mkdir %q"
	end

	assert( cmd_template )
	return function(path)
		return os.execute(cmd_template:format(path))
	end
end)()

--[[
-- Returns whether a command exists in PATH.
--
-- Under Unix, if a relative or absolute path is given (instead of a bare
-- command name), return true if that file exists and is executable. Under
-- Windows, a similar behaviour occurs, since "." belongs to PATH.
--]]
local cmdExists = (function()
	if hostIsUnix() then
		return function(name)
			return os.execute(("which %q &>/dev/null"):format(name))
		end
	else
		return function(name)
			return os.execute(("where %q > nul 2> nul"):format(name))
		end
	end
end)()

local extract = (function()
	--[[
	-- Command formatting string.
	--]]
	local cmd_template

	if hostIsUnix() then
		--[[
		-- Mac OS X and almost all Linux distributions have an unzip
		-- command. The only Linux distros which don't come with unzip
		-- are "power user" distros, in which either:
		-- + The user will have installed unzip already, maybe as a dependency.
		-- + The user will know how to install it otherwise.
		--]]
		if cmdExists("unzip") then
			cmd_template = "unzip -o %q -d %q"
		elseif cmdExists("7z") then
			cmd_template = "7z -y x %q -o %q"
		end
	else
		local bundled_extractor = catfile(props.tooldir, "7z.exe")
		cmd_template = bundled_extractor.." -y x %q -o %q"
	end

	if cmd_template then
		return function(file, folder)
			mkdir(folder)
			return os.execute(cmd_template:format(file, folder))
		end
	else
		return error("Unable to determine an extraction method.")
	end
end)()

local copyDirTree = (function()
	local cmd_template

	if hostIsUnix() then
		cmd_template = "cp -r %q %q"
	else
		cmd_template = "xcopy /s /t /e %q %q"
	end
	return function(src, dest)
		return os.execute(cmd_template:format(src, dest))
	end
end)()


-------------------------------------------------------------------------


props = os_properties[_OPTIONS.os]
props.outdir = catfile("..", "..", "ds_mod_tools_out")
props.skuoutdir = catfile(props.outdir, props.dir, "mod_tools")
props.tooldir = catfile("..", "tools", "bin", props.dir)

apps = 
{
	'scml', 
	'png', 
	'autocompiler', 
	--'textureconverter'
}

libs = 
{
	--texturelib = { include_lib = true },
	modtoollib = { include_lib = false },
}

solution('mod_tools')
	configurations { "debug", "release" }
	location ( catfile(props.outdir, "proj") )
	flags { "Symbols", "NoRTTI", "NoEditAndContinue", "NoExceptions", "NoPCH" }
	includedirs { "lib", catfile("..", "lib") }
  	targetdir ( props.skuoutdir )

    configuration { "debug" }
        defines { "DEBUG", "_CRT_SECURE_NO_WARNINGS" }
   	configuration { "release" }
        defines { "RELEASE", "_CRT_SECURE_NO_WARNINGS" }
        flags { "Optimize" }	

	for k, app in pairs(apps) do	
	   	project(app)
			kind "ConsoleApp"
			language "C++"   	   
	      	files { catfile("app", app, "**.h"), catfile("app", app, "**.hpp"), catfile("app", app, "**.cpp") }	 
	      	for lib, settings in pairs(libs) do
	      		links{ lib }
	      	end
	end

	for lib, settings in pairs(libs) do	
	   	project(lib)
			kind "StaticLib"
			language "C++"   	   
	      	files { catfile("lib", lib, "**.h"), catfile("lib", lib, "**.hpp"), catfile("lib", lib, "**.cpp") }
	      	if settings.include_lib then
	      		includedirs { catfile("lib", lib) }
	      	end
	end


extract(catfile("..", "pkg", "tst", "wand.zip"), catfile(props.outdir, "dont_starve", "mods"))
copyDirTree(catfile("..", "pkg", "cmn", "mod_tools"), catfile(props.outdir, props.dir))

if targetIsWindows() then
	extract(catfile("..", "pkg", props.dir, "mod_tools.zip"), catfile(props.outdir, props.dir))
	extract(catfile("..", "pkg", props.dir, "Python27.zip"), catfile(props.skuoutdir, "buildtools", props.pythondir))
end
