solution("scml")
   configurations { "debug", "release" }
    
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