ds_mod_tools
============

This is a fork from the source code for the 'Don't Starve Mod Tools' available on Steam Workshop.

It is a port from the original mod tools for Linux and Mac. Additionally, it properly calculates the bounding boxes of objects with custom animation compiled from .scml files.

Instructions
============

Run premake4 on 'src/premake4.lua' to generate the projects in the 'build/proj' folder.
The output is built to 'build/win32', 'build/osx' or 'build/linux'.

Todo
============
- calculate proper pivots in scml so it's no longer necessary to name things 'build' or 'player_build'
- setup better way for specifying frame numbers and durations
- add support to scml for hidden timeline objects
