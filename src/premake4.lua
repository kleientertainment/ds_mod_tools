os_properties = 
{
	windows = { dir = 'win32', pkg_dirs = {"cmn", "win32"}, pythondir = 'windows' },
	macosx = { dir = 'osx', pkg_dirs = {"cmn", "unix"}, pythondir = 'osx' },
	linux = { dir = 'linux', pkg_dirs = {"cmn", "unix"}, pythondir = 'linux' },
}


-------------------------------------------------------------------------

-- Concatenates path components using the native directory separator.
local function catfile(...)
	local t = {...}
	local u = {}

	for i = 1, select('#', ...) do
		local v = t[i]
		if type(v) == "string" and #v > 0 and (i == 1 or v ~= ".") then
			table.insert(u, v)
		end
	end

	return table.concat(u, package.config:sub(1,1))
end

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
			return "osx"
		end
	end
end



if _OPTIONS.os == nil then
	io.write("Target OS not specified. Assuming it's the host OS.", "\n")
	_OPTIONS.os = deduceOS()
end
if _OPTIONS.os == nil then
   error('Please specify your target os!')
elseif os_properties[_OPTIONS.os] == nil then
	error('Unsupported os!')
end


props = os_properties[_OPTIONS.os]
props.outdir = catfile("..", "build")
props.skuoutdir = catfile(props.outdir, props.dir, "mod_tools")
props.tooldir = catfile("..", "tools", "bin", props.dir)


-------------------------------------------------------------------------
--[[
-- Utility functions.
--]]

local function runcmd(cmd)
	io.write(cmd, "\n")
	return os.execute(cmd)
end

--[[
-- To be used as: definesMacros { "foo", bar = "qux" },
-- i.e. array entries are simple defines, hash ones define values.
--
-- See http://sourceforge.net/p/premake/bugs/275/ for why it's needed.
--]]
local definesMacros = (function()
	local function macro(name, value)
		local ret = tostring(name)
		if value then
			if type(value) == "string" then
				if _ACTION == "vs2010" then
					ret = ret..('="%s"'):format(value)
				else
					ret = ret..('=\\"%s\\"'):format(value)
				end
			else
				ret = ret..'='..tostring(value)
			end
		end
		return ret
	end

	return function(t)
		local u = {}
		for k, v in pairs(t) do
			local M
			if type(k) == "number" then
				M = macro(v)
			else
				M = macro(k, v)
			end
			table.insert(u, M)
		end
		return defines(u)
	end
end)()

local function targetIsUnix()
	return _OPTIONS.os == "linux" or _OPTIONS.os == "osx"
end

local function targetIsWindows()
	return _OPTIONS.os == "windows"
end

assert( targetIsUnix() or targetIsWindows() )

local hostIsUnix, hostIsWindows
do
	local host = assert( deduceOS(), "Unable to infer the host OS." )

	local function True() return true end
	local function False() return false end

	if host == "windows" then
		hostIsUnix, hostIsWindows = False, True
	else
		hostIsUnix, hostIsWindows = True, False
	end
end

-- Creates a directory, including intermediate ones.
local mkdir = (function()
	--[[
	-- Command formatting string.
	--]]
	local cmd_template
	if hostIsUnix() then
		cmd_template = 'mkdir -p %q'
	else
		cmd_template = 'mkdir "%s"'
	end

	assert( cmd_template )
	return function(path)
		return runcmd(cmd_template:format(path))
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
			return runcmd(('which %q &>/dev/null'):format(name))
		end
	else
		return function(name)
			return runcmd(('where "%s" > nul 2> nul'):format(name))
		end
	end
end)()

local rmDirTree = (function()
	--[[
	-- Command formatting string.
	--]]
	local cmd_template

	if hostIsUnix() then
		cmd_template = 'rm -rf %q'
	else
		cmd_template = 'rmdir /s /q "%s"'
	end

	return function(path)
		return runcmd( cmd_template:format(path) )
	end
end)()

local function rmSecondArgument(x, y)
	return rmDirTree(y)
end

local extract = (function()
	if _ACTION == "clean" then
		return rmSecondArgument
	end

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
			cmd_template = "unzip -q -o %q -d %q"
		elseif cmdExists("7z") then
			cmd_template = '7z -y x %q "-o%s"'
		end
	else
		local bundled_extractor = catfile(props.tooldir, "7z.exe")
		cmd_template = bundled_extractor..' -y x "%s" "-o%s"'
	end

	if cmd_template then
		return function(file, folder)
			mkdir(folder)
			return runcmd(cmd_template:format(file, folder))
		end
	else
		return error("Unable to determine an extraction method.")
	end
end)()

local copyDirTree = (function()
	if _ACTION == "clean" then
		return rmSecondArgument
	end

	local cmd_template

	if hostIsUnix() then
		cmd_template = "cp -rT %q %q"
	else
		cmd_template = 'xcopy /e /y "%s" "%s"'
	end
	return function(src, dest)
		mkdir(dest)
		return runcmd(cmd_template:format(src, dest))
	end
end)()


-------------------------------------------------------------------------


mkdir( props.outdir )

--[[
-- Maps subdirectories of pkg/{pkg_dir}/ to subdirectories of mod_tools.
--]]
pkg_map = {
	[catfile("mod_tools")] = catfile("."),
	[catfile("Python27")] = catfile("buildtools", props.pythondir, "Python27"),
}


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

	definesMacros { PYTHONDIR = props.pythondir }

    configuration { "debug" }
        definesMacros { "DEBUG", "_CRT_SECURE_NO_WARNINGS" }
   	configuration { "release" }
        definesMacros { "RELEASE", "_CRT_SECURE_NO_WARNINGS" }
        flags { "Optimize" }	

	for k, app in pairs(apps) do	
	   	project(app)
			kind "ConsoleApp"
			language "C++"   	   
	      	files { "app/"..app.."/**.h", "app/"..app.."/**.hpp", "app/"..app.."/**.cpp" }	 
	      	for lib, settings in pairs(libs) do
	      		links{ lib }
	      	end
	end

	for lib, settings in pairs(libs) do	
	   	project(lib)
			kind "StaticLib"
			language "C++"   	   
	      	files { "lib/"..lib.."/**.h", "lib/"..lib.."/**.hpp", "lib/"..lib.."/**.cpp" }
	      	if settings.include_lib then
	      		includedirs { "lib/"..lib }
	      	end
	end


extract(catfile("..", "pkg", "tst", "wand.zip"), catfile(props.outdir, "dont_starve", "mods"))

for _, src_dir in ipairs(props.pkg_dirs) do
	local src_base = catfile("..", "pkg", src_dir)
	local dest_base = props.skuoutdir
	for src_subdir, dest_subdir in pairs(pkg_map) do
		local src = catfile(src_base, src_subdir)
		local dest = catfile(dest_base, dest_subdir)
		if os.isdir(src) then
			copyDirTree(src, dest)
		end
	end
end
