if _OPTIONS.os == nil then
   error('Please specify your target os!')
end

apps = {'scml'}

for k, app in pairs(apps) do
	solution(app)
	   configurations { "debug", "release" }
	   location "../gen/proj"

	   configuration("windows")
	      targetdir( "../win32/compilers" )
	   configuration("macosx")
	      targetdir( "../osx/compilers" )
	   configuration("linux")
	      targetdir( "../linux/compilers" )

	   flags { "Symbols", "NoRTTI", "NoEditAndContinue", "NoExceptions", "NoPCH" }

	   project(app)
	      kind "ConsoleApp"
	      language "C++"
	      files { "apps/"..app.."/**.h", "apps/"..app.."/**.cpp" }      
	 
	      configuration "debug"
	         defines { "DEBUG" }
	 
	      configuration "release"
	         defines { "RELEASE" }
	         flags { "Optimize" }      	
end