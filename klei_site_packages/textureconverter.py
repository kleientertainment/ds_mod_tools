import sys, os, tempfile, shutil
from PIL import Image
import util
import subprocess

TOOL_PATH = os.path.join( os.path.dirname( __file__ ), "../../../../../../tools/bin" )
TEXTURE_CONVERTER = os.path.abspath( os.path.join( TOOL_PATH, "TextureConverter.exe" ) )

def WinConvert( src_filenames, dest_filename, texture_format="bc3", no_premultiply=False, force=False, platform='opengl',
             generate_mips=False, width=None, height=None,
             verbose=False, ignore_exceptions=False):
    is_newer = False

    # If a list is passed in, concatenate the filenames with semi-colon separators, otherwise just use the filename
    src_filename_str = None
    if isinstance( src_filenames, list ):
        src_filename_str = ';'.join( src_filenames )
        for filename in src_filenames:
            is_newer = is_newer or util.IsFileNewer( filename, dest_filename )
    else:
        src_filename_str = src_filenames
        is_newer = is_newer or util.IsFileNewer( src_filename_str, dest_filename )

    if force or is_newer:
        cmd_list = [ TEXTURE_CONVERTER,
                          '--swizzle',
                          '--format ' + texture_format,
                          '--platform ' + platform,
                          '-i ' + src_filename_str,
                          '-o ' + dest_filename,
                    ]

        if generate_mips:
            cmd_list.append( '--mipmap' )

        if not no_premultiply:
            cmd_list.append( '--premultiply' )

        if width:
            cmd_list.append( '-w {}'.format( width ) )
        if height:
            cmd_list.append( '-h {}'.format( height ) )

        cmd = " ".join( cmd_list )
        if verbose:
            print( cmd )
        if subprocess.call( cmd_list ) != 0:
            sys.stderr.write( "Error attempting to convert {} to {}\n".format( src_filenames, dest_filename ) )
            sys.stderr.write( cmd + "\n" )
            if not ignore_exceptions:
                raise

KTECH_LOCATION = "ktech"

texture_format_translation = {
    'bc1': 'dxt1',
    'bc2': 'dxt3',
    'bc3': 'dxt5',
    'rgb': 'rgb',
    'argb': 'rgba',
    }

def UnixConvert( src_filenames, dest_filename, texture_format="bc3", no_premultiply=False, force=False, platform='opengl',
             generate_mips=False, width=None, height=None,
             verbose=False, ignore_exceptions=False):
    is_newer = False

    assert texture_format_translation[texture_format], "Invalid texture format {}".format(texture_format)
    texture_format = texture_format_translation[texture_format]
    
    # ktech has multiplatform texture support as a stub, but disabled since only opengl is used.
    # (the support was built in, but the option to define in as a program parameter was left commented out)
    assert platform == "opengl", "Invalid platform {}".format(platform)

    # If a list is passed in, concatenate the filenames with semi-colon separators, otherwise just use the filename
    src_filename_str = None

    if isinstance( src_filenames, list ):
        src_filename_str = ','.join( src_filenames )
        for filename in src_filenames:
            is_newer = is_newer or util.IsFileNewer( filename, dest_filename )
    else:
        src_filename_str = src_filenames
        is_newer = is_newer or util.IsFileNewer( src_filename_str, dest_filename )

    if force or is_newer:
        cmd_list = [ KTECH_LOCATION,
                        '--quiet',
                        '--compression ' + texture_format,
                   ]

        if not generate_mips:
            cmd_list.append( '--no-mipmaps' )

        if no_premultiply:
            cmd_list.append( '--no-premultiply' )

        if width:
            cmd_list.append( '--width {}'.format( width ) )
        if height:
            cmd_list.append( '--height {}'.format( height ) )

        cmd_list.append( '--' )

        cmd_list.append( src_filename_str )
        cmd_list.append( dest_filename )

        cmd = " ".join( cmd_list )
        if verbose:
            print( cmd )
        if subprocess.call( cmd_list ) != 0:
            sys.stderr.write( "Error attempting to convert {} to {}\n".format( src_filenames, dest_filename ) )
            sys.stderr.write( cmd + "\n" )
            if not ignore_exceptions:
                raise

def Convert( src_filenames, dest_filename, texture_format="bc3", no_premultiply=False, force=False, platform='opengl',
             generate_mips=False, width=None, height=None,
             verbose=False, ignore_exceptions=False):
    if sys.platform == "win32":
        return WinConvert( src_filenames, dest_filename, texture_format, no_premultiply, force, platform, generate_mips, width, height, verbose, ignore_exceptions)
    else:
        return UnixConvert( src_filenames, dest_filename, texture_format, no_premultiply, force, platform, generate_mips, width, height, verbose, ignore_exceptions)


def GenerateMips( im ):
    mips = []
    w, h = im.size

    while w >= 1 or h >= 1:
        mips.append( im )

        w /= 2
        h /= 2

        im = im.resize( ( max( w, 1 ), max( h, 1 ) ), Image.ANTIALIAS )

    return mips


def SaveImagesToTemp( images, basename ):
    tempdir = tempfile.mkdtemp()
    idx = 0

    filenames = []
    for image in images:
        name = "{0}{1}.png".format( basename, idx )

        filename = os.path.join( tempdir, name )
        filenames.append( filename )
        image.save( filename  )

        idx += 1

    return ( tempdir, filenames )


def MipAndConvert( im, dest_filename, platform='opengl', texture_format="bc3", no_premultiply = False, force=False, ignore_exceptions=False ):
    if isinstance( im, str ):
        im = Image.open( im )
    mips = GenerateMips( im )

    tempdir, filenames = SaveImagesToTemp( mips, "mip" )
    try:
        Convert( src_filenames=filenames, dest_filename=dest_filename, texture_format=texture_format, platform=platform,
                no_premultiply=no_premultiply, force=force, ignore_exceptions=ignore_exceptions )
    finally:
        if os.path.exists( tempdir ):
            shutil.rmtree( tempdir )



