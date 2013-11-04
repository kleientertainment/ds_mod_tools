# The keys in the SizeMap are used to test against the filename and ALSO the
# name of the prefab. This is important for things like the hud, where you want
# everything in the hud to keep 1:1 sizing
#
# Filenames override prefab settings

import collections

def DefaultSize():
	return 1.0

SizeMap = collections.defaultdict( DefaultSize )
SizeMap[ "hud" ] = 1.0
SizeMap[ "frontend" ] = 1.0
SizeMap[ "carrot" ] = .5
SizeMap[ "fireflies" ] = .5
SizeMap[ "flies" ] = .5

SizeMap[ "ice_over.zip" ] = 1.0
SizeMap[ "vig.zip" ] = 1.0
SizeMap[ "clouds_ol.zip" ] = 0.25
SizeMap[ "fire_over.zip" ] = 0.5
SizeMap[ "button.zip" ] = 1.0
SizeMap[ "button_small.zip" ] = 1.0
SizeMap[ "button_long.zip" ] = 1.0

SizeMap[ "creepy_hands.zip" ] = 1.0
SizeMap[ "generating_cave.zip" ] = 1.0
SizeMap[ "generating_world.zip" ] = 1.0
