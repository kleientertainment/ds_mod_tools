import sys
import optparse
import glob
import os
import string
import tempfile
import shutil
import zipfile
from pipelinetools import *
from objloader import *
import struct

def ProcessFile(file, options):
    #figure out some name stuff
    basename = os.path.splitext(os.path.basename(file))[0]
    print "Compiling " + basename
    basedir = GetBaseDirectory(file, "intermediates")
    tooldir = VerifyDirectory(basedir,"tools")
    outdir = VerifyDirectory(basedir, "data/models")
    
    tempdir = tempfile.mkdtemp()

    tempvbname = os.path.join(tempdir, "model.vb")
    inzip = zipfile.ZipFile(file, "r")
    objfile = ObjFile(inzip.open("model.obj"))
   
    if options.bigendian:
        endianstring = ">"
    else:
        endianstring = "<"
    
    outfile = open(tempvbname, 'wb')
    for face in objfile.faces:
        for vert in face:
            pos = objfile.vertices[vert[0]-1]
            tex = objfile.texcoords[vert[1]-1]
            norms = objfile.normals[vert[2]-1]
            packed = struct.pack(endianstring + "ffffffff", pos[0], pos[1], pos[2], norms[0], norms[1], norms[2], tex[0], tex[1])
            outfile.write(packed)
    outfile.close()
    
    
    inzip.extract("texture.png", tempdir)
    #convert the image file....
    
    outzip = zipfile.ZipFile(os.path.join(outdir, basename+".zip"), "w")
    for f in [ tempvbname, os.path.join(tempdir, "texture.png") ]:
        outzip.write(f, os.path.relpath(f, tempdir), zipfile.ZIP_DEFLATED)
    shutil.rmtree(tempdir)
    
    
def main(argv):

    try:
        import psyco
        psyco.full()
    except ImportError:
        pass

    parser = optparse.OptionParser(description='compiles zipped intermediate model to a platform-specific format in data/models/*.zip')
    parser.add_option('-b', '--bigendian', help="bigendian", action="store_true", dest="bigendian")
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