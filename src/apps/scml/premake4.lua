if _OPTIONS.os == nil then
   error('Please specify your target os!')
end

solution("scml")
   configurations { "debug", "release" }
   location "../../../gen/proj"

   configuration("windows")
      targetdir( "../../../win32/compilers" )
   configuration("macosx")
      targetdir( "../../../osx/compilers" )
   configuration("linux")
      targetdir( "../../../linux/compilers" )

   flags { "Symbols", "NoRTTI", "NoEditAndContinue", "NoExceptions", "NoPCH" }

   project "scml"
      kind "ConsoleApp"
      language "C++"
      files { "**.h", "**.cpp" }      
 
      configuration "debug"
         defines { "DEBUG" }
 
      configuration "release"
         defines { "RELEASE" }
         flags { "Optimize" }      