import argparse, shutil
import re
import struct
import traceback
import xml.dom.minidom
import zipfile
import math
import os
import sys
from PIL import Image
from PIL import ImageDraw
import optimizeimage
import tempfile
import re

from StringIO import StringIO
from collections import namedtuple

from klei import atlas, textureconverter

ZIP_ZERO_TIME = ( 1980, 0, 0, 0, 0, 0 )
BUILDVERSION = 6
#BUILD format 6
# 'BILD'
# Version (int)
# total symbols;
# total frames;
# build name (int, string)
# num materials
#   material texture name (int, string)
#for each symbol:
#   symbol hash (int)
#   num frames (int)
#       frame num (int)
#       frame duration (int)
#       bbox x,y,w,h (floats)
#       vb start index (int)
#       num verts (int)

# num vertices (int)
#   x,y,z,u,v,w (all floats)
#
# num hashed strings (int)
#   hash (int)
#   original string (int, string)

ANIMVERSION = 4
#ANIM format 4
# 'ANIM'
# Version (int)
# total num element refs (int)
# total num frames (int)
# total num events (int)
# Numanims (int)
#   animname (int, string)
#   validfacings (byte bit mask) (xxxx dlur)
#   rootsymbolhash int
#   frame rate (float)
#   num frames (int)
#       x, y, w, h : (all floats)
#       num events(int)
#           event hash
#       num elements(int)
#           symbol hash (int)
#           symbol frame (int)
#           folder hash (int)
#           mat a, b, c, d, tx, ty, tz: (all floats)
#
# num hashed strings (int)
#   hash (int)
#   original string (int, string)


FACING_RIGHT = 1
FACING_UP = 2
FACING_LEFT = 4
FACING_DOWN = 8

def strhash(str, hashcollection):
    hash = 0
    for c in str:
        v = ord(c.lower())
        hash = (v + (hash << 6) + (hash << 16) - hash) & 0xFFFFFFFFL
    hashcollection[hash] = str
    return hash

def get_z_index(element):
    return int(element.attributes["z_index"].value)

EXPORT_DEPTH = 10
def ExportAnim(endianstring, xmlstr, outzip, ignore_exceptions):
    hashcollection = {}
    doc = xml.dom.minidom.parseString(xmlstr)
    outfile = StringIO()

    outfile.write(struct.pack(endianstring +  'cccci', 'A', 'N', 'I', 'M', ANIMVERSION))
    outfile.write(struct.pack(endianstring + 'I', len(doc.getElementsByTagName("element"))))
    outfile.write(struct.pack(endianstring + 'I', len(doc.getElementsByTagName("frame"))))
    outfile.write(struct.pack(endianstring + 'I', len(doc.getElementsByTagName("event"))))
    outfile.write(struct.pack(endianstring + 'I', len(doc.getElementsByTagName("anim"))))

    def LocalExport( anim_node ):
        name = anim_node.attributes["name"].value.encode('ascii')
        
        dirs = (re.search("(.*)_up\Z", name),
                re.search("(.*)_down\Z", name),
                re.search("(.*)_side\Z", name),
                re.search("(.*)_left\Z", name),
                re.search("(.*)_right\Z", name))
        
        facingbyte = FACING_RIGHT | FACING_LEFT | FACING_UP | FACING_DOWN
        
        if dirs[0]:
            name = dirs[0].group(1)
            facingbyte = FACING_UP
        elif dirs[1]:
            name = dirs[1].group(1)
            facingbyte = FACING_DOWN
        elif dirs[2]:
            name = dirs[2].group(1)
            facingbyte = FACING_LEFT | FACING_RIGHT
        elif dirs[3]:
            name = dirs[3].group(1)
            facingbyte = FACING_LEFT
        elif dirs[4]:
            name = dirs[4].group(1)
            facingbyte = FACING_RIGHT
        
        root = anim_node.attributes["root"].value.encode('ascii')
        num_frames = len(anim_node.getElementsByTagName("frame"))
        frame_rate = int(anim_node.attributes["framerate"].value)

        outfile.write(struct.pack(endianstring + 'i' + str(len(name)) + 's', len(name), name))
        outfile.write(struct.pack(endianstring + 'B', facingbyte))
        outfile.write(struct.pack(endianstring + 'I', strhash(root, hashcollection)))
        outfile.write(struct.pack(endianstring + 'fI', float(frame_rate), num_frames))
        for frame_node in anim_node.getElementsByTagName("frame"):
            outfile.write(struct.pack(endianstring + 'ffff',
                float(frame_node.attributes["x"].value),
                float(frame_node.attributes["y"].value),
                float(frame_node.attributes["w"].value),
                float(frame_node.attributes["h"].value)))
            num_events = len(frame_node.getElementsByTagName("event"))                
            outfile.write(struct.pack(endianstring + 'I', num_events))
            
            for event_node in frame_node.getElementsByTagName("event"):
                outfile.write(struct.pack(endianstring + 'I', strhash(event_node.attributes["name"].value.encode('ascii'), hashcollection)))
            
            elements = frame_node.getElementsByTagName("element")
            try:
                elements = sorted(elements, key=get_z_index)
            except:
                pass
            
            num_elements = len(elements)
            outfile.write(struct.pack(endianstring + 'I', num_elements))
            
            eidx = 0
            for element_node in elements:
                outfile.write(struct.pack(endianstring + 'I', strhash(element_node.attributes["name"].value.encode('ascii'), hashcollection)))
                outfile.write(struct.pack(endianstring + 'I', int(element_node.attributes["frame"].value)))
                layername = element_node.attributes["layername"].value.encode('ascii').split('/')[-1]
                outfile.write(struct.pack(endianstring + 'I', strhash(layername, hashcollection)))
                        
                z = (eidx/float(num_elements)) * float(EXPORT_DEPTH) - EXPORT_DEPTH*.5
                outfile.write(struct.pack(endianstring + 'fffffff',
                    float(element_node.attributes["m_a"].value),
                    float(element_node.attributes["m_b"].value),
                    float(element_node.attributes["m_c"].value),
                    float(element_node.attributes["m_d"].value),
                    float(element_node.attributes["m_tx"].value),
                    float(element_node.attributes["m_ty"].value),
                    z))

                eidx += 1

    nodes = doc.getElementsByTagName("anim")
    if not ignore_exceptions:
        print("Exporting Animation")
        for anim_node in nodes:
            LocalExport( anim_node )
    else:
        for anim_node in nodes:
            LocalExport( anim_node )

    #write out a lookup table of the pre-hashed strings
    outfile.write(struct.pack(endianstring + 'I', len(hashcollection)))
    for hash_idx,name in hashcollection.iteritems():
        outfile.write(struct.pack(endianstring + 'I', hash_idx))
        outfile.write(struct.pack(endianstring + 'i' + str(len(name)) + 's', len(name), name))

    info = zipfile.ZipInfo( "anim.bin", date_time=ZIP_ZERO_TIME )
    info.compress_type = zipfile.ZIP_DEFLATED
    info.external_attr = 0644 << 16L
    outzip.writestr(info, outfile.getvalue())

Vert = namedtuple("Vertex", "x y z u v w")

#ims: list of (name, PIL image)
#outname: output name for atlas images
#outzip: open zipfile to contain output images

#returns dest, atlases
#dest = {image_name: [ (origbbox, destbbox, destatlasidx) ]
#atlases = {index : (name, image) ]

def AtlasImages(ims, outname, outzip, maxtexturesize=2048, antialias=True, platform='opengl', textureformat='bc3', hardalphatextureformat='bc1', force=False, ignore_exceptions=False):
    dest = {}
    atlases = {}

    if len(ims) > 0:

        #do the atlasing
        atlases = atlas.Atlas(ims=ims, outname=outname, max_size=maxtexturesize, scale_factor=SCALE_FACTOR, ignore_exceptions=ignore_exceptions, force_square=results.square)

        temp_dir = tempfile.mkdtemp()
        try:
            #convert all of the result images to the game format, and save them to the zip file

            for idx, atlasdata in atlases.items():
                tex_filename = atlasdata.mips[0].name + ".tex"
                dest_filename = os.path.join( temp_dir, tex_filename )

                mip_filenames = [ os.path.join( temp_dir, atlasdata.mips[0].name + "_" + str( mip_idx ) + ".png" ) for mip_idx in range( len( atlasdata.mips ) ) ]

                valid_filenames = []
                textureformat = textureformat if antialias else hardalphatextureformat

                for mip_idx in range( len( atlasdata.mips ) ):

                    mip = atlasdata.mips[ mip_idx ]
                    mip_filename = mip_filenames[ mip_idx ]
                    
                    filter_func = Image.ANTIALIAS if antialias else Image.NEAREST
                    mip_img = mip[1]

                    width = mip_img.size[0] * SCALE_FACTOR
                    height = mip_img.size[1] * SCALE_FACTOR

                    if width < 1 and height < 1:
                        break
					
                    mip_img = mip_img.resize( (int(math.ceil(width)), int(math.ceil(height))), filter_func )

                    with file( mip_filename, "wb" ) as src_file:
                        valid_filenames.append( mip_filename )
                        mip_img.save( src_file, format="PNG" )
                        src_file.close()

                textureconverter.Convert(
                        src_filenames=valid_filenames,
                        dest_filename=dest_filename,
                        texture_format=textureformat,
                        platform=platform,
                        force=force,
                        generate_mips=True,
                        ignore_exceptions=ignore_exceptions)

                info = zipfile.ZipInfo( tex_filename, date_time=ZIP_ZERO_TIME )
                info.compress_type = zipfile.ZIP_DEFLATED
                info.external_attr = 0644 << 16L
                outzip.writestr( info, open( dest_filename, 'rb' ).read() )

                for mip_filename in valid_filenames:
                    os.remove( mip_filename )
                os.remove( dest_filename )
        finally:
            shutil.rmtree( temp_dir )

    return atlases


#destlist is a list of AtlasDest namedtuples

def AddVertsToVB( image_name, region_type, atlases, xoff, yoff, zoff, VB, sampleroffset = 0 ):
    numverts = 0

    for atlas_idx in range( len( atlases ) ):
        atlas = atlases[atlas_idx]
        bboxes = atlas.bboxes
        src_images = atlas.src_images

        sampler = atlas_idx + sampleroffset

        imsizex = float(atlas.mips[0].im.size[0])
        imsizey = float(atlas.mips[0].im.size[1])

        for img in src_images:
            if img.name != image_name:
                continue

            src_regions = None

            if region_type == "alpha":
                src_regions = img.regions.alpha
            elif region_type == "opaque":
                src_regions = img.regions.opaque

            if not src_regions:
                continue

            dest_bbox = bboxes[ img.name ]

            # new_img = Image.new( "RGBA", img.size )
            # for region in img.regions.opaque:
            #     cropped = img.crop( ( region.x, region.y, region.x + region.w, region.y + region.h ) )
            #     new_img.paste( cropped, ( region.x, region.y, region.x + region.w, region.y + region.h ) )
            # new_img.save( "test.png" )
            # sys.exit ( 5 )

            for region in src_regions:
                assert region.x <= img.size[0]
                assert region.y <= img.size[1]
                assert region.x + region.w <= img.size[0]
                assert region.y + region.h <= img.size[1]

                left = xoff + region.x
                right = left + region.w

                top = yoff + region.y
                bottom = top + region.h

                z = zoff

                umin = max( 0.0, min( 1.0, ( dest_bbox.x + region.x ) / imsizex ) )
                umax = max( 0.0, min( 1.0, ( dest_bbox.x + region.x + region.w ) / imsizex ) )
                vmin = max( 0.0, min( 1.0, 1 - ( dest_bbox.y + region.y ) / imsizey ) )
                vmax = max( 0.0, min( 1.0, 1 - ( dest_bbox.y + region.y + region.h ) / imsizey ) )

                #print( umin, umax, vmin, vmax )

                assert 0 <= umin and umin <= 1
                assert 0 <= umax and umax <= 1
                assert 0 <= vmin and vmin <= 1
                assert 0 <= vmax and vmax <= 1


                VB.append(Vert(left, top , z, umin, vmin, sampler))
                VB.append(Vert(right, top, z, umax, vmin, sampler))
                VB.append(Vert(left, bottom, z, umin, vmax, sampler))
                VB.append(Vert(right, top, z, umax, vmin, sampler))
                VB.append(Vert(right, bottom, z, umax, vmax, sampler))
                VB.append(Vert(left, bottom, z, umin, vmax, sampler))

                numverts += 6

    return numverts

def ExportBuild(endianstring, inzip, buildxml, outzip, antialias, platform, textureformat, hardalphatextureformat, force, ignore_exceptions):
    hashcollection = {}

    doc = xml.dom.minidom.parseString(buildxml)
    
    imnames = {frame.attributes["image"].value.encode('ascii') for frame in doc.getElementsByTagName("Frame")}

    images = []

    for imname in imnames:
        img = Image.open( StringIO( inzip.read( imname + ".png" ) ) )
        img.name = imname
        img.regions = optimizeimage.GetImageRegions(img, 32)

        images.append( img )
    
    atlases = AtlasImages( images, "atlas", outzip, 2048, antialias, platform, textureformat, hardalphatextureformat, force, ignore_exceptions)

    opaqueverts = []
    alphaverts = []

    outfile = StringIO()

    outfile.write(struct.pack(endianstring +  'cccci', 'B', 'I', 'L', 'D', BUILDVERSION))

    outfile.write(struct.pack(endianstring + 'I', len(doc.getElementsByTagName("Symbol"))))
    outfile.write(struct.pack(endianstring + 'I', len(doc.getElementsByTagName("Frame"))))

    buildname = doc.getElementsByTagName("Build")[0].attributes["name"].value.encode('ascii')
    buildname = os.path.splitext(buildname)[0]
    outfile.write(struct.pack(endianstring + 'i' + str(len(buildname)) + 's', len(buildname), buildname))

    #write out the number of atlases:
    outfile.write(struct.pack(endianstring + 'I', len(atlases)))
    for atlas_idx in range( len( atlases ) ):
        atlasdata = atlases[ atlas_idx ]
        mip = atlasdata.mips[0]
        name = mip.name + ".tex"

        outfile.write(struct.pack(endianstring + 'i' + str(len(name)) + 's', len(name), name))

    symbol_nodes = sorted(doc.getElementsByTagName("Symbol"), key=lambda x: strhash(x.attributes["name"].value.encode('ascii'), hashcollection))

    for symbol_node in symbol_nodes:#doc.getElementsByTagName("Symbol"):
        symbolname = symbol_node.attributes["name"].value.encode('ascii')
        outfile.write(struct.pack(endianstring + 'I', strhash(symbolname, hashcollection)))
        outfile.write(struct.pack(endianstring + 'I', len(symbol_node.getElementsByTagName("Frame"))))

        for frame_node in symbol_node.getElementsByTagName("Frame"):
            framenum = int(frame_node.attributes["framenum"].value)
            duration = int(frame_node.attributes["duration"].value)

            w = float(frame_node.attributes["w"].value)
            h = float(frame_node.attributes["h"].value)
            x = float(frame_node.attributes["x"].value)
            y = float(frame_node.attributes["y"].value)

            imagename = frame_node.attributes["image"].value.encode('ascii')

            xoff = x - w/2
            yoff = y - h/2
            z = 0

            alphaidx = len(alphaverts)
            alphacount = AddVertsToVB( imagename, "alpha", atlases, xoff, yoff, z, alphaverts )
            
            #opaqueidx = len(opaqueverts)
            alphacount += AddVertsToVB( imagename, "opaque", atlases, xoff, yoff, z, alphaverts )

            outfile.write(struct.pack(endianstring + 'I', framenum))
            outfile.write(struct.pack(endianstring + 'I', duration))
            outfile.write(struct.pack(endianstring + 'ffff', x,y,w,h))
            
            outfile.write(struct.pack(endianstring + 'I', alphaidx))
            outfile.write(struct.pack(endianstring + 'I', alphacount))
            #outfile.write(struct.pack(endianstring + 'I', opaqueidx))
            #outfile.write(struct.pack(endianstring + 'I', opaquecount))

    outfile.write(struct.pack(endianstring + 'I', len(alphaverts)))
    for vert in alphaverts:
        outfile.write(struct.pack(endianstring + 'ffffff', vert.x, vert.y, vert.z, vert.u, vert.v, vert.w))
    #outfile.write(struct.pack(endianstring + 'I', len(opaqueverts)))
    #for vert in opaqueverts:
    #    outfile.write(struct.pack(endianstring + 'ffffff', vert.x, vert.y, vert.z, vert.u, vert.v, vert.w))
    
    #write out a lookup table of the pre-hashed strings
    outfile.write(struct.pack(endianstring + 'I', len(hashcollection)))
    for hash_idx,name in hashcollection.iteritems():
        outfile.write(struct.pack(endianstring + 'I', hash_idx))
        outfile.write(struct.pack(endianstring + 'i' + str(len(name)) + 's', len(name), name))

    info = zipfile.ZipInfo( "build.bin", date_time=ZIP_ZERO_TIME )
    info.compress_type = zipfile.ZIP_DEFLATED
    info.external_attr = 0644 << 16L
    outzip.writestr(info, outfile.getvalue())

SCALE_FACTOR = 1.0
VERBOSITY = 0

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Export an animation from xml/png intermediates to game data.')
    parser.add_argument('infile', action="store")
    parser.add_argument('--bigendian', action="store_true", default=False)
    parser.add_argument('--scale', action='store', default=1.0, type=float, required=False)
    parser.add_argument('--verbosity', action='store', default=0, type=int, required=False)
    parser.add_argument('--skipantialias', action='store_true')
    parser.add_argument('--textureformat', default='bc3')
    parser.add_argument('--hardalphatextureformat', default='bc1')
    parser.add_argument('--platform', default='opengl')
    parser.add_argument('--force', action='store_true')
    parser.add_argument('--ignoreexceptions', action='store_true')
    parser.add_argument('--square', action='store_true')
    parser.add_argument('--outputdir', default='data' )
    results = parser.parse_args()

    try:
        endianstring = "<"
        if results.bigendian:
            endianstring = ">"

        if results.scale:
            SCALE_FACTOR = results.scale

        if results.verbosity:
            VERBOSITY = results.verbosity

        path, base_name = os.path.split(results.infile)
        base_name, ext = os.path.splitext(base_name)
        path = os.path.abspath( path )

        if ext != ".zip":
            sys.stderr.write( "I only accept zip files, and you tried to make me process " + results.infile + "\n" )
            sys.exit()

        data_path = os.path.abspath(os.path.join(path, "..", results.outputdir))
        zip_file = zipfile.ZipFile(results.infile, "r")

        outbuff = StringIO()
        outzip = zipfile.ZipFile(outbuff, mode='w')

        if "animation.xml" in zip_file.namelist():
            animxml = zip_file.read("animation.xml")
            ExportAnim(endianstring, animxml, outzip, results.ignoreexceptions)

        if "build.xml" in zip_file.namelist():
            buildxml = zip_file.read("build.xml")
            ExportBuild( endianstring, zip_file, buildxml, outzip, not results.skipantialias, results.platform, results.textureformat, results.hardalphatextureformat, results.force, results.ignoreexceptions )

        outzip.close()

        outfilename = os.path.join( data_path, "anim", base_name + ".zip" )
        if not os.path.exists( os.path.join( data_path, "anim" ) ):
            os.makedirs( os.path.join( data_path, "anim" ) )
        f = open(outfilename, "wb")
        f.write(outbuff.getvalue())
        f.close()

        if not results.ignoreexceptions:
            try:
                import pysvn
                client = pysvn.Client()            
                client.add( outfilename )
            except:
                pass

            try:
                client = pysvn.Client()
                client.add_to_changelist( outfilename, 'Export ' + base_name)
            except:
                pass
    except: # catch *all* exceptions
        e = sys.exc_info()[1]
        sys.stderr.write( "Error Exporting {}\n".format(results.infile) + str(e) )
        traceback.print_exc(file=sys.stderr)
        if not results.ignoreexceptions:
            print("There was an export error!")
            exit(-1)


