ds_mod_tools
============

This is the source code for the 'Don't Starve Mod Tools' available on Steam Workshop.

Instructions
============

Run premake4 on 'src/premake4.lua' to generate the projects in the '/gen' folder.
The output is built to '/win32', '/osx' or '/linux'

Todo
============
- port autocompiler to osx/linux
- port scml to osx/linux
- port png to osx/linux
- port textureconverter to osx/linux
- add python to osx/linux
- add common script directory which gets deployed to each platform
- reorganize repo so that it works better with github
- calculate proper pivots in scml so it's no longer necessary to name things 'build' or 'player_build'
- calculate proper bounding boxes in scml to fix selection issues
- setup better way for specifying frame numbers and durations