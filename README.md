ds_mod_tools
============

This is a fork from the source code for the 'Don't Starve Mod Tools' available on Steam Workshop.

It is a port from the original mod tools for Linux and Mac. In also improves on the original animation (i.e., SCML) compiler in mod tools primarily
+ properly calculating the bounding boxes of animation frames;
+ determining the correct frame to use in a timeline with variable images (i.e., proper correlation of animation frame elements to build symbol frames).

The Linux and Mac porting, as well as the improvements mentioned above, were done by @simplex. @DeathDisciple was responsible for getting the tools compiling once again under Windows, after the Unix port.

Compilation-time dependencies
============

This project uses [Premake](http://industriousone.com/premake/download) to generate the build files, so it must be installed.

Additionally, a C++ compiler is required. For Windows, either Visual Studio or MinGW may be used. For Mac, it is necessary to download and install the XCode package from Apple. For Linux, gcc and make (GNU make, also known as gmake) are required (you probably already have them).

Runtime dependencies
============

Under Windows, the tools have no runtime dependencies other than those bundled within the tools.

For Mac and Linux, a system wide installation of Python 2.7 is assumed, as well as the Python Imaging Library (PIL) or its backwards-compatible fork (Pillow). A system wide installation of the TEX converter ktech in [ktools
](https://github.com/nsimplex/ktools#installation-from-source) is also assumed.

Compilation instructions
============

For Windows, run the src/premake4.bat script to generate a Visual Studio 2010 project in build/proj/. Then open this project in Visual Studio and build it. For MinGW, see the note below.

For Mac and Linux, enter `src/` and run `premake.sh`, then enter the directory `build/proj` with a terminal and enter the command
```
$ make
```
(without the prefixing '$')

The output is built to `build/win32`, `build/osx` or `build/linux`.

To customize the build files placed in build/proj, instead of running src/premake.bat or src/premake.sh the premake4 program may be run directly from a terminal (cmd.exe, under Windows). For the full list of options, run `premake4 --help`, but the most significant scenario is generating GNU make files for compilation using MinGW, under Windows. This can be done via a
```
premake4 gmake
```

Usage
============

The tools were designed for being run by the autocompiler, which in turn was designed under the assumption the mod tools would be placed in a directory alongside Don't Starve's installation directory with the same structure as when installing them via Steam. In order to use the autocompiler, a folder structure such as this must be replicated (for example, under Windows, the folder placed at build/win32/mod_tools after compilation must be relocated in such a way that mod_tools/../../dont_starve is Don't Starve's installation directory).

Alternatively, the scml compiler may be used directly. I takes two arguments: the first one is the path of the scml file to build and the second one is the path to the directory of the mod in which to place the final product (an anim/ folder is created within the mod's if it doesn't exist).

Todo
============
- setup better way for specifying frame numbers and durations
