import sys
import optparse
import glob
import os
import string
import tempfile
import shutil
import zipfile
import textureconverter
from pipelinetools import *

def MakeObjQuad(origsize, trimsize, scale, depth, plane, outfilename):
    p0 = [ (trimsize[0] - origsize[0]/2)*scale, (-trimsize[3]+origsize[1]/2)*scale, depth ]
    p1 = [ (trimsize[0] - origsize[0]/2)*scale, (-trimsize[1]+origsize[1]/2)*scale, depth ]
    p2 = [ (trimsize[2] - origsize[0]/2)*scale, (-trimsize[1]+origsize[1]/2)*scale, depth ]
    p3 = [ (trimsize[2] - origsize[0]/2)*scale, (-trimsize[3]+origsize[1]/2)*scale, depth ]

    
    if plane == "XZ":
        p0[1], p0[2] = p0[2], p0[1]
        p1[1], p1[2] = p1[2], p1[1]
        p2[1], p2[2] = p2[2], p2[1]
        p3[1], p3[2] = p3[2], p3[1]
        
        
    f = open(outfilename, "w")
    f.write ( 'v %(x)f %(y)f %(z)f\n'% {'x':p0[0], 'y':p0[1], 'z':p0[2]})
    f.write ( 'v %(x)f %(y)f %(z)f\n'% {'x':p1[0], 'y':p1[1], 'z':p1[2]})
    f.write ( 'v %(x)f %(y)f %(z)f\n'% {'x':p2[0], 'y':p2[1], 'z':p2[2]})
    f.write ( 'v %(x)f %(y)f %(z)f\n'% {'x':p3[0], 'y':p3[1], 'z':p3[2]})
    
    if plane=="XY":
        f.write ( 'vn 0 0 1\n')
    elif plane=="XZ":
        f.write ( 'vn 0 1 0\n')
    f.write ( 'vt 1 0\n')
    f.write ( 'vt 1 1\n')
    f.write ( 'vt 0 1\n')
    f.write ( 'vt 0 0\n')
    #f.write ( 'f 1/1/1 2/2/1 3/3/1 4/4/1\n')
    f.write ( 'f 1/1/1 2/2/1 3/3/1 \n')
    f.write ( 'f 1/1/1 3/3/1 4/4/1 \n')

   
def ProcessFile(file, options):
    #figure out some name stuff
    basename = os.path.splitext(os.path.basename(file))[0]
    print "Exporting " + file

    basedir = GetBaseDirectory(file, "contentsrc")
    tooldir = VerifyDirectory(basedir, "tools")
    outdir = VerifyDirectory(basedir, "intermediates/models")
    tempdir = tempfile.mkdtemp()
    
    #trim the file, save the size, save the image
    origsize, trimsize, trimmed = TrimImage(file)
    tempimagename = os.path.join(tempdir, "texture.png")
    tempobjname = os.path.join(tempdir, "model.obj")
    trimmed.save(tempimagename)

    tex_filename = os.path.join( os.path.dirname( tempimagename ), "texture.tex" )
    textureconverter.Convert( tempimagename, tex_filename )

    MakeObjQuad(origsize, trimsize, options.scale, options.depth, options.plane, tempobjname)

    zip_filename = os.path.join(outdir, basename+".zip")
    print zip_filename
    outzip = zipfile.ZipFile( zip_filename, "w")
    for f in [ tex_filename, tempobjname ]:
        archive_name = os.path.relpath(f, tempdir)
        outzip.write(f, archive_name, zipfile.ZIP_DEFLATED)
    outzip.close()

    shutil.rmtree(tempdir)
    
    
def main(argv):

    try:
        import psyco
        psyco.full()
    except ImportError:
        pass

    parser = optparse.OptionParser(description='Process image files into intermediate obj+cropped texture pairs')
    parser.add_option('-s', '--scale', help="scale applied to the model to bring it from pixel space", default=1, type=float)
    parser.add_option('-d', '--depth', help="depth to use", default=0, type=float)
    parser.add_option('-p', '--plane', help="plane to use (XY or XZ)", default='XY', type='choice', choices=['XY', 'XZ'])
    options, arguments = parser.parse_args()

    #generate a list of unique files from the arguments
    files = list(set(sum([glob.glob(x) for x in arguments], [])))
    
    if len(files) == 0:
        print ("No input files specified")
        sys.exit(2)
        
    for file in files:
        ProcessFile(file, options)
        
if __name__ == "__main__":
    main(sys.argv[1:])
