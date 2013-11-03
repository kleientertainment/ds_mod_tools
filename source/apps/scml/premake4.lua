solution("scml")
   configurations { "debug", "release" }
   location "../../../gen/proj"
   targetdir "../../../win32/compilers"   
   flags { "Symbols", "NoRTTI", "NoEditAndContinue", "NoExceptions", "NoPCH" }

   project "scml"
      kind "ConsoleApp"
      language "C++"
      files { "**.h", "**.cpp" }      
 
      configuration "debug"
         defines { "DEBUG" }
 
      configuration "release"
         defines { "NDEBUG" }      