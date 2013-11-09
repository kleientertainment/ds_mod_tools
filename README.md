ds_mod_tools
============

This is the source code for the 'Don't Starve Mod Tools' available on Steam Workshop.

Instructions
============

Run premake4 on 'src/premake4.lua' to generate the projects in the '../ds_mod_tools_out/proj' folder.
The output is built to '../ds_mod_tools_out/win32', '../ds_mod_tools_out/osx' or '../ds_mod_tools_out/linux'.  This is so that github doesn't slow down checking for diffs on files which won't be committed.

Todo
============
- port autocompiler to osx/linux
- port scml to osx/linux
- port png to osx/linux
- port textureconverter to osx/linux
- add python to osx/linux
- add common script directory which gets deployed to each platform
- calculate proper pivots in scml so it's no longer necessary to name things 'build' or 'player_build'
- calculate proper bounding boxes in scml to fix selection issues
- setup better way for specifying frame numbers and durations
- add support to scml for hidden timeline objects