import sys, os, argparse, glob
import xml.etree.ElementTree as ET
import ResizeInfo, ExportOptions
from klei import util
import subprocess

SCRIPT_DIR = os.path.dirname( __file__ )

sys.path.append( SCRIPT_DIR )

class Asset:
    def __init__( self, filename, asset_type ):
        self.File = filename
        self.Type = asset_type

class Prefab:
    def __init__( self, name, assets = [] ):
        self.Name = name
        self.Assets = assets

class Prefabs:
    def __init__( self, filename ):
        self.Prefabs = self.ParsePrefabs( filename )

    def GetAssetType( self, asset_type ):
        asset_type = asset_type.upper()
        assets = []
        
        for prefab in self.Prefabs:
            for asset in prefab.Assets:
                if asset.Type == asset_type:
                    assets.append( asset )

        return assets

    def ParsePrefabs( self, filename ):
        tree = ET.parse( filename )
        prefabs = tree.findall( "Defs/Prefab" )

        prefab_list = []
        for prefab in prefabs:
            name = prefab.attrib[ 'name' ]
            assets = [ Asset( asset.attrib[ 'file' ], asset.attrib[ 'type' ].upper() ) for asset in prefab.findall( "Assets/Asset" ) ]
            prefab_list.append( Prefab( name, assets ) )

        return prefab_list

    def GetOwner( self, asset_filename ):
        for prefab in self.Prefabs:
            for asset in prefab.Assets:
                if os.path.basename( asset.File ) == asset_filename:
                    return prefab
        return None
        

def ConvertAnim(params):
    anim, prefabs, args = params
    anim_path = anim

    anim = os.path.basename(anim.strip())
    prefab_owner = prefabs.GetOwner( anim )
    scale = ResizeInfo.DefaultSize()
    if prefab_owner:
        scale = ResizeInfo.SizeMap[ prefab_owner.Name ]
        #print (prefab_owner.Name, scale)

    # Filename scaling overrides general prefab settings
    anim_filename = os.path.basename( anim )
    anim_name, ext = os.path.splitext( anim_filename )
    keys_to_try = [ anim, anim_filename, anim_name ]
    for key in keys_to_try:
        scale = ResizeInfo.SizeMap.get( key, scale )

    additional_options = ExportOptions.Options.get( anim_name, [] )
    cmd_args = [
        os.path.join( SCRIPT_DIR, r"..\buildtools\windows\python27\python.exe" ),
        os.path.join( SCRIPT_DIR, r"..\tools\scripts\buildanimation.py" ),
        "--scale=" + str( scale )
    ]
    
    additional_args = ( 'platform', 'textureformat', 'hardalphatextureformat' )
    for arg in additional_args:
        try:
            val = getattr( args, arg )
            cmd_args.append( '--{}={}'.format( arg, val ) )
        except:
            pass

    additional_flags = ( 'force', 'square', 'ignoreexceptions' )
    for flag in additional_flags:
        try:
            val = getattr( args, flag )
            if val:
                cmd_args.append( '--' + flag )
        except:
            pass

    os.chdir( SCRIPT_DIR )

    src_file = anim_path    
    anim_dir = os.path.normpath( os.path.join("..", args.outputdir, "anim"))
    if not os.path.exists(anim_dir):
        os.makedirs(anim_dir)
    dest_file = os.path.join(anim_dir, anim)

    if args.force or util.IsFileNewer( src_file, dest_file ):
        cmd_args += additional_options
        cmd_args.append( src_file )
        cmd_args.append( '--outputdir=' + args.outputdir )

        print( '\tExporting ' + anim )
        return subprocess.call(cmd_args)
    else:
        return 0

if __name__ == "__main__":
    CWD = os.getcwd()

    # Now do the export
    parser = argparse.ArgumentParser( description = "Animation batch exporter script" )
    parser.add_argument( '--platform', default='opengl' )
    parser.add_argument( '--textureformat', default='bc3' )
    parser.add_argument( '--hardalphatextureformat', default='bc1' )
    parser.add_argument( '--force', action='store_true' )
    parser.add_argument( '--square', action='store_true' )
    parser.add_argument( '--ignoreexceptions', action='store_true' )
    parser.add_argument( 'anims', type=str, nargs='?', help="List of anim filenames to be exported" )
    parser.add_argument( '--outputdir', default='data' )
    parser.add_argument( '--prefabsdir', default='' )
    parser.add_argument( '--skip_update_prefabs', action='store_true' )
    args = parser.parse_args()

    if len( args.prefabsdir ) == 0:
        args.prefabsdir = os.path.join( SCRIPT_DIR, "..", args.outputdir )

    prefabs_file = os.path.abspath( os.path.normpath( os.path.join( CWD, args.prefabsdir, "prefabs.xml" ) ) )
    prefabs = Prefabs( prefabs_file )
    anims = []
    if args.anims:
        anims = [ args.anims ]
    else:
        anims = [ os.path.basename( anim.File ) for anim in prefabs.GetAssetType( "anim" ) ]

    # sorted, unique anims
    anims = sorted( set( anims ) )
    anims = [ ( anim, prefabs, args ) for anim in anims ]

    from multiprocessing import Pool
    pool = Pool(processes=1)
    pool.map( ConvertAnim, anims )

    # Ensure that the prefabs.xml file is up to date
    if not args.skip_update_prefabs:
        os.chdir( os.path.join( SCRIPT_DIR, ".." ) )
        os.system( ' '.join( ( 'updateprefabs.bat', args.outputdir ) ) )

    os.chdir( CWD )
