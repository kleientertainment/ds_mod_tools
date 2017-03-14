import sys, os, math

from PIL import Image

from collections import namedtuple
import xml.etree.ElementTree as ET

BBox = namedtuple("Bbox", 'x y w h')
AtlasData = namedtuple("Atlas", "src_images, mips, bboxes")

def NextMultipleOf(n, target):
    mod = n % target
    if mod == 0:
        return n
    return n + (target-mod)

def GetDim(ims, alignto, maxtexturesize=2048, scale_factor=1):
    area = reduce(lambda tally, next : tally + next.size[0]*next.size[1]*scale_factor*scale_factor, ims, 0)
    maxdim = NextMultipleOf(max([ max(img.size[0], img.size[1])*scale_factor for img in ims] ), alignto)
    dim = min( maxtexturesize, max( math.pow(2, math.ceil(math.log(math.sqrt(area*1.25),2))), math.pow(2, math.ceil(math.log(maxdim,2)))))
    #print("GETDIM:", area, maxdim, dim)

    return dim/scale_factor

def BBoxIntersects(bb1, bb2):
    if bb2.x >= bb1.x + bb1.w or \
       bb2.x + bb2.w <= bb1.x or \
       bb2.y + bb2.h <= bb1.y or \
       bb2.y >= bb1.y + bb1.h:
        return False
    return True


def TryInsertImage(w,h, fblist, atlassize):
    align = 4
    can_insert = False
    mylist = [fb for fb in fblist]

    x = 0
    y = 0

    while y + h < atlassize:
        min_y = None
        ytestbb = BBox(0, y, atlassize, h)
        templist = [fb for fb in mylist if BBoxIntersects(fb.bbox, ytestbb)]
        while x + w <= atlassize:
            testbb = BBox(x,y,w,h)
            intersects = False
            for fb in templist:
                if BBoxIntersects(fb.bbox, testbb):
                    x = NextMultipleOf(fb.bbox.x + fb.bbox.w, align)
                    if not min_y:
                        min_y = fb.bbox.h+fb.bbox.y
                    else:
                        min_y = min(min_y, fb.bbox.h+fb.bbox.y)
                    intersects = True
                    break
            if not intersects:
                return BBox(x,y,w,h)
        if min_y:
            y = max(NextMultipleOf(min_y, align), y + align)
        else:
            y += align
        x = 0
        #mylist = [fb for fb in mylist if BBoxIntersects(fb.bbox, BBox(0, y, atlassize, atlassize-y))]

    return None


def Clamp( lower, upper, val ):
    return max( lower, min( upper, val ) )

def GenerateXMLTree( texture_filename, texture_size, bboxes, offset_amount=None ):
    root = ET.Element( "Atlas" )
    tex_elem = ET.SubElement( root, "Texture"  )
    tex_elem.set( "filename", os.path.basename( texture_filename ) )

    elem_root = ET.SubElement( root, "Elements" )

    # pull in the UVs by a half pixel from the edge to avoid some sampling issues, unless told otherwise
    offset_amount_x = offset_amount if offset_amount != None else 0.5
    offset_amount_y = offset_amount if offset_amount != None else 0.5

    border_uv_offset = ( offset_amount_x / texture_size[0], offset_amount_y / texture_size[1] )

    for filename, bbox in bboxes.items():
        elem = ET.SubElement( elem_root, "Element" )
        elem.set( "name", filename )

        u1 = Clamp( 0.0, 1.0, bbox.x / float( texture_size[0] ) + border_uv_offset[0] )
        v1 = Clamp( 0.0, 1.0, 1.0 - ( bbox.y + bbox.h ) / float( texture_size[1] ) + border_uv_offset[1] )

        u2 = Clamp( 0.0, 1.0, ( bbox.x + bbox.w ) / float( texture_size[0] ) - border_uv_offset[0] )
        v2 = Clamp( 0.0, 1.0, 1.0 - bbox.y / float( texture_size[1] ) - border_uv_offset[1] )

        elem.set( "u1", str( u1 ) )
        elem.set( "v1", str( v1 ) )

        elem.set( "u2", str( u2 ) )
        elem.set( "v2", str( v2 ) )

    tree = ET.ElementTree( root )

    return tree

OutImage = namedtuple("OutImage", "name im")
FullBox = namedtuple("FullBox", "im, outidx, bbox, name")

#ims: list of subimages
#outname: prefix name for output image

#returns dest, atlases
#dest = {image_name: [ (origbbox, destbbox, destatlasidx) ]
#atlases = {index : (name, image) ]

def Atlas(ims, outname, max_size=2048, scale_factor=1, ignore_exceptions=False, minimize_num_textures=True, force_square=False ):
    blocksize = 4
    dim = GetDim(ims, blocksize, max_size, scale_factor)
    size = (dim,dim)
    
    #sort by image area
    ims = sorted(ims, key = lambda im : im.size[1]*im.size[0], reverse=True)

    #Full boxes are areas where we have placed images in the atlas
    fullboxes = {0 :(size,[])}

    #Do the actual atlasing by sticking the largest images we can have into the smallest valid free boxes
    source_idx = 0

    def LocalAtlas( im, source_idx ):
        if im.size[0] > size[0] or im.size[1] > size[1]:
            sys.stderr.write( "Error: image " + im.name + " is larger than the atlas size!\n" )
            sys.exit(2)

        inserted = False
        for idx, fb in fullboxes.items():
            atlassize = dim
            fblist = fb[1]

            insertbbox = TryInsertImage(im.size[0], im.size[1], fblist, atlassize)
            if insertbbox:
                inserted = True
                fblist.append( FullBox( im, idx, insertbbox, im.name ) )
                break

        if not inserted:
            numoutimages = len(fullboxes)

            newsize = GetDim(ims[source_idx:], blocksize, max_size, scale_factor)
            fullboxes[numoutimages] = ((newsize,newsize), [FullBox(im, numoutimages, BBox(0,0,im.size[0], im.size[1]), im.name )])

        return source_idx + 1


    if ignore_exceptions:
        for im in ims:
            source_idx = LocalAtlas( im, source_idx )
    else:
        print("Atlasing")
        for im in ims:
            source_idx = LocalAtlas( im, source_idx )
    
    #now that we've figured out where everything goes, make the output images and blit the source images to the appropriate locations

    atlases = {}
    for idx, fb in fullboxes.items():
        w = int(fb[0][0])
        h = int(fb[0][1])

        if not force_square:
            #figure out if we can reduce our w or h:    
            sz = fb[0][0]
            fblist = fb[1]
            maxy = 0
            maxx = 0
            for b in fblist:
                fbmaxy = b.bbox.y+b.bbox.h
                fbmaxx = b.bbox.x+b.bbox.w
                maxy = max(maxy, fbmaxy)
                maxx = max(maxx, fbmaxx)
            if maxy <= h/2:
                h = int(h/2)
            if maxx <= w/2:
                w = int(w/2)

        #now generate mips and such...
        mips = []

        divisor = 1

        contained_images = {}

        # Generate mips and their positions
        while w >= 1 or h >= 1:    
            outim = OutImage( "{0}-{1}".format(outname, idx), Image.new( "RGBA", ( w, h ) ) )

            for b in fb[1]:
                b_w, b_h = b.im.size
                b_w, b_h = b_w / divisor, b_h / divisor
                if b_w > 0 and b_h > 0:
                    resized_b = b.im.resize( ( b_w, b_h ), Image.ANTIALIAS )
                    b_x, b_y = b.bbox.x / divisor, b.bbox.y / divisor
                    outim[1].paste( resized_b, ( b_x, b_y ) )
                    contained_images[ b.im ] = True

            mips.append( outim )

            divisor = divisor << 1

            if w == 1 and h == 1:
                break

            w = max( 1, w >> 1 )
            h = max( 1, h >> 1 )

        bboxes = { b.name : b.bbox for b in fb[1] }
        atlases[idx] = AtlasData( contained_images.keys(), mips, bboxes )

    return atlases
