import zipfile, sys, os, glob 
import xml.etree.ElementTree as ET
from clint.textui import progress
from collections import defaultdict

anim_map = defaultdict( list )

for zipfilename in progress.bar( glob.glob( "*.zip" ) ):
    try:
        with zipfile.ZipFile( zipfilename, "r" ) as zf:
            root = ET.fromstring( zf.read( "animation.xml" ) )
            for anim in root.findall( "anim" ):
                animname = anim.attrib[ 'name' ]
                rootname = anim.attrib[ 'root' ]

                key = ( animname, rootname )
                anim_map[ key ].append( zipfilename )
    except:
        pass

invalid = False
for key, datalist in anim_map.iteritems():
    if len( datalist ) > 1:
        print key
        print datalist
        print
        invalid = True

if invalid:
    sys.exit( 255 )
