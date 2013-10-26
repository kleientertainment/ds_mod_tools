# The multilayer texture combiner takes in a list of textures from the bottom
# layer upwards and puts them into appropriate channels of a texture.
#
# The game shader must then be coded to intelligently read the individual
# channels and use the information as if it was from a separate texture.

from PIL import Image

def BuildMultilayer( src_filenames ):
    assert 1 <= len( src_filenames ) and len( src_filenames ) <= 4
    imgs = [ Image.open( filename ) for filename in src_filenames ]
    expected_size = imgs[0].size

    image_mode = "RGBA"

    for img in imgs:
        assert img.size == expected_size
        assert img.mode == image_mode

    alpha_bands = []
    for img in imgs:
        img.load()
        R, G, B, A = img.split()
        new_img = Image.merge( "L", (A,) )
        alpha_bands.append( new_img )

    empty_band = Image.new( "L", expected_size, 255 )
    for i in xrange( len( image_mode ) - len( alpha_bands ) ):
        alpha_bands.append( empty_band )

    return Image.merge( image_mode, alpha_bands )
