ds_mod_tools
============

This is the source code for the 'Don't Starve Mod Tools' available on Steam Workshop.

Instructions
============

Run premake4 on 'src/premake4.lua' to generate the projects in the 'build/proj' folder.
The output is built to 'build/win32', 'build/osx' or 'build/linux'.

Todo
============
- port textureconverter to osx/linux or include ktech
- calculate proper pivots in scml so it's no longer necessary to name things 'build' or 'player_build'
- calculate proper bounding boxes in scml to fix selection issues
- setup better way for specifying frame numbers and durations
- add support to scml for hidden timeline objects
