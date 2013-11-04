if _OPTIONS.os == nil then
   error('Please specify your target os!')
end

apps = {'scml'}

for k, app in pairs(apps) do
	solution(app)
	   configurations { "debug", "release" }
	   location "../gen/proj"

	   configuration("windows")
	      targetdir( "../win32/Don't Starve Mod Tools/compilers" )
	   configuration("macosx")
	      targetdir( "../osx/Don't Starve Mod Tools/compilers" )
	   configuration("linux")
	      targetdir( "../linux/Don't Starve Mod Tools/compilers" )

	   flags { "Symbols", "NoRTTI", "NoEditAndContinue", "NoExceptions", "NoPCH" }

	   project(app)
	      kind "ConsoleApp"
	      language "C++"
	      files { "app/"..app.."/**.h", "app/"..app.."/**.cpp" }      
	 
	      configuration "debug"
	         defines { "DEBUG" }
	 
	      configuration "release"
	         defines { "RELEASE" }
	         flags { "Optimize" }      	
end