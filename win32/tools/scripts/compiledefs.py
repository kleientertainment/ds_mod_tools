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
    outdir = VerifyDirectory(basedir, "data/defs")
    outfilename = os.path.join(outdir, basename + ".xml")
    shutil.copyfile(file, outfilename)
    shutil.copyfile(file, outfilename)
    
    
def main(argv):

    try:
        import psyco
        psyco.full()
    except ImportError:
        pass

    parser = optparse.OptionParser(description='compiles defs (currently just copy the xml)')
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