from Crypto.Random import random # This is supposed to be a cryptographically strong version of Python's standard "random" module
import os, sys, argparse
from clint import textui

parser = argparse.ArgumentParser()
parser.add_argument( '-keylen', type=int, action="store", required=True )
parser.add_argument( '-numkeys', type=int, nargs=1, required=True )
parser.add_argument( '-characterset', action="store", required=True )
args = parser.parse_args()

if os.path.exists( args.characterset ):
    args.characterset = open( args.characterset, "r" ).read().strip()

sys.stderr.write( "The key space has a total of {0} keys".format( pow( len( args.characterset ), args.keylen ) ) )
keylen = args.keylen

for i in textui.progress.bar( xrange( args.numkeys[0] ) ):
    key = ''.join( random.sample( args.characterset, keylen ) )
    key = '-'.join( [ key[i:i+4] for i in xrange( 0, keylen, 4 ) ] )
    print key
