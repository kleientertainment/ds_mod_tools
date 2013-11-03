solution("scml")
   configurations { "debug", "release" }
   location "../../../gen_proj"
   targetdir "../../../win32/compilers"

   project "scml"
      kind "ConsoleApp"
      language "C++"
      files { "**.h", "**.cpp" }
 
      configuration "debug"
         defines { "DEBUG" }
         flags { "Symbols" }
 
      configuration "release"
         defines { "NDEBUG" }
         flags { "Symbols" }         