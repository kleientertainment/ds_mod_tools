solution("scml")
   configurations { "debug", "release" }
   location "../../../gen_proj"
    
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