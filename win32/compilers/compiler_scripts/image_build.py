# This is based on build.py from contentsrc/images

import sys, os, argparse, glob, re, tempfile, shutil

from klei import textureconverter, atlas, imgutil, util
from PIL import Image
from properties import ImageProperties

def is_power2(num):
    return num != 0 and ((num & (num - 1)) == 0)

def ExportImage(filename):
    path, base_name = os.path.split(filename)
    base_name, ext = os.path.splitext(base_name)
    path = os.path.abspath( path )
    dest_filename = os.path.join(path, base_name+".tex")

    if util.IsAnyFileNewer( filename, dest_filename ) or args.force:
        im = Image.open( filename )
        sz = im.size
        print( "\tExporting " + filename + " to " + dest_filename)
        assert sz[0] %4 == 0 and sz[1] %4 == 0, "Image " + filename + " has a dimension that is not a multiple of 4"

        skipmip = not is_power2(sz[0]) or not is_power2(sz[1])

        textureconverter.Convert( src_filenames=filename, dest_filename=dest_filename,
                force=args.force, texture_format=args.textureformat, platform=args.platform,
                generate_mips=not skipmip, no_premultiply=im.mode=='RGB', ignore_exceptions=args.ignoreexceptions )

#parse args
parser = argparse.ArgumentParser( description = "Animation batch exporter script" )
parser.add_argument( '--textureformat', default='bc3' )
parser.add_argument( '--platform', default='opengl' )
parser.add_argument( '--force', action='store_true' )
parser.add_argument( '--hardalphatextureformat', default='bc1' )
parser.add_argument( '--square', action='store_true' )
parser.add_argument( 'images', type=str, nargs='*' )
parser.add_argument( '--ignoreexceptions', action='store_true' )
parser.add_argument( '--outputdir', default='data' )
args = parser.parse_args()
in_images = args.images


ROOT = os.path.abspath( "../.." )

class BBox:
    x = 0
    y = 0
    w = 1
    h = 1

def process_file_to_atlas():
    for image in in_images:
        path, base_name = os.path.split(image)
        image_base_name, image_ext = os.path.splitext( base_name )
        dest_filename = os.path.join( path, image_base_name + ".tex" )
        xml_filename = os.path.join( path, image_base_name + ".xml" )
        
        #sys.stderr.write(image+"\n")
        #sys.stderr.write(dest_filename+"\n")
        #sys.stderr.write(xml_filename+"\n")
        #if not util.IsAnyFileNewer( image, [dest_filename, xml_filename] ) or args.force:
            #continue

        working_dir = os.path.join( tempfile.gettempdir(), image_base_name )
        try:
            if os.path.exists( working_dir ):
                shutil.rmtree( working_dir )
            os.makedirs( working_dir )

            border_size = 0

            images = []
            im = imgutil.OpenImage( image, size=None, border_size=border_size )
            im.name = image_base_name + ".tex"
            images.append( im )

            sz = im.size
            npot = not is_power2(sz[0]) or not is_power2(sz[1])
            textureformat = 'bc3' #'argb' if npot else 'bc3'

            sys.stderr.write("\nAtlasing "+image+" npot: "+str(npot)+"\n")

            atlas_data = atlas.Atlas( images, image_base_name, minimize_num_textures=True, ignore_exceptions=args.ignoreexceptions, force_square=args.square )
            assert len( atlas_data ) == 1
            page = atlas_data[0]

            mip_filenames = []
            for mip_idx in range( len( page.mips ) ):
                mip = page.mips[mip_idx]

                mip_filename = mip.name + "-mip{}.png".format( mip_idx )
                full_mip_filename = os.path.join( working_dir, mip_filename )
                mip_filenames.append( full_mip_filename )
                mip.im.save( full_mip_filename )

            textureconverter.Convert(
                    src_filenames=mip_filenames, dest_filename=dest_filename,
                    platform=args.platform, texture_format=textureformat, force=args.force,
                    ignore_exceptions=args.ignoreexceptions, generate_mips=True) #, no_premultiply=im.mode=='RGB' )
                    
            tex_filename = os.path.relpath( dest_filename, ROOT ).replace( "\\", "/" )
            xml = atlas.GenerateXMLTree( tex_filename, page.mips[0].im.size, page.bboxes, border_size + 0.5 )
            xml.write( xml_filename )
        finally:
            if os.path.exists( working_dir ):
                shutil.rmtree( working_dir )


process_file_to_atlas()

# This function is not used and so is probably inaccurate. If you ever want to process an entire directory
# to a single atlas this is a good starting spot though.
def process_dirs_to_atlas():
    for dirname in [ d for d in os.listdir( '.' ) if os.path.isdir( d ) and not d in excluded_dirs ]:
        filenames = glob.glob( os.path.join( dirname, "*.png" ) )
        dest_filename = os.path.join( out_path, dirname + ".tex" )
        xml_filename = os.path.join( out_path, dirname + ".xml" )

        if util.IsAnyFileNewer( filenames, [ dest_filename, xml_filename ] ) or args.force:
            print( "Processing " + dirname )

            working_dir = os.path.join( tempfile.gettempdir(), dirname )
            try:
                if os.path.exists( working_dir ):
                    shutil.rmtree( working_dir )
                os.makedirs( working_dir )

                size = None
                border_size = 0

                if dirname in ImageProperties:
                    width = ImageProperties[dirname].get( "width", None )
                    height = ImageProperties[dirname].get( "height", None )
                    border_size = ImageProperties[dirname].get( "border", 0 )

                    if width != None and height != None:
                        size = ( width, height )

                images = []
                for filename in filenames:
                    im = imgutil.OpenImage( filename, size=size, border_size=border_size )
                    filename, ext = os.path.splitext( os.path.basename( filename ) )
                    im.name = filename + ".tex" 
                    images.append( im )

                atlas_data = atlas.Atlas( images, dirname, minimize_num_textures=True, ignore_exceptions=args.ignoreexceptions, force_square=args.square )
                assert len( atlas_data ) == 1
                page = atlas_data[0]

                mip_filenames = []
                for mip_idx in range( len( page.mips ) ):
                    mip = page.mips[mip_idx]

                    mip_filename = mip.name + "-mip{}.png".format( mip_idx )
                    full_mip_filename = os.path.join( working_dir, mip_filename )
                    mip_filenames.append( full_mip_filename )
                    mip.im.save( full_mip_filename )

                textureconverter.Convert(
                        src_filenames=mip_filenames, dest_filename=dest_filename,
                        platform=args.platform, texture_format=args.textureformat, force=args.force,
                        ignore_exceptions=args.ignoreexceptions)
                        
                tex_filename = os.path.relpath( dest_filename, ROOT ).replace( "\\", "/" )
                xml = atlas.GenerateXMLTree( tex_filename, page.mips[0].im.size, page.bboxes, border_size + 0.5 )
                xml.write( xml_filename )

            finally:
                if os.path.exists( working_dir ):
                    shutil.rmtree( working_dir )

