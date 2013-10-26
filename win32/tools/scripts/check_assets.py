import sys, os, argparse

buildtools_dir = os.path.normpath( os.path.join( os.path.dirname( __file__ ), "../../buildtools/scripts" ) )
sys.path.append( buildtools_dir )

import PackageData
parser = argparse.ArgumentParser()
parser.add_argument( '--outputdir', default='data' )
args = parser.parse_args()

print

missing_file = False
for filename, arcname in PackageData.CommonFiles( data_dir = args.outputdir ):
    if not os.path.exists( filename ):
        sys.stderr.write( "{} is referenced in prefabs.xml but does not exist!\n".format( filename ) )
        missing_file = True
    else:
        path, fn = os.path.split( filename )
        files = os.listdir( path )
        if fn not in files:
            sys.stderr.write( "\n---------------------------------------------------------------------------\n" )
            sys.stderr.write( "FILENAME MISMATCH - Possibly case?\n" )
            sys.stderr.write( "{} - specified in prefab\n".format( filename ) )
            sys.stderr.write( "\n---------------------------------------------------------------------------\n" )
            raise

assert missing_file == False

