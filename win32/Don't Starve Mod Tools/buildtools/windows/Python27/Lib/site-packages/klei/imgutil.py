from PIL import Image, ImageOps

def Expand( im, border_size ):
    new_img = ImageOps.expand( im, border = border_size )

    for i in range( border_size ):
        left = border_size - i
        right = border_size + im.size[0] + i
        top = border_size - i
        bottom = border_size + im.size[1] + i - 1

        top_line = new_img.crop( ( left, top, right, top + 1 ) )
        bottom_line = new_img.crop( ( left, bottom, right, bottom + 1 ) )

        new_img.paste( top_line, ( left, top - 1, right, top ) )
        new_img.paste( bottom_line, ( left, bottom + 1, right, bottom + 2 ) )

        left_line = new_img.crop( ( left, top - 1, left + 1, bottom + 2 ) )
        right_line = new_img.crop( ( right-1, top - 1, right, bottom + 2 ) )

        new_img.paste( left_line, ( left - 1, top - 1, left, bottom + 2 ) )
        new_img.paste( right_line, ( right, top - 1, right + 1, bottom + 2 ) )

    return new_img

def OpenImage( filename, size=None, border_size = 0 ):
    im = Image.open( filename )

    if size != None:
        im = im.resize( size, Image.ANTIALIAS )

    im = Expand( im, border_size )

    return im

