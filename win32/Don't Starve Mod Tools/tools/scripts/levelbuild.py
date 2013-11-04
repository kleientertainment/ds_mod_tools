import sys
import string
import xml.dom.minidom
import argparse
import struct

VERSION = 1
# LEVEL format 1:
# "LEVL"
# VERSION (int)
# scenario file (int + string)
# num entities (int)
# for each entity:
#   prefab name (int + string)
#   name (int + string)
#   number of tags (int)
#       each tag (int)
#   number of components (int)
#   for each component:
#       Component ID (int)
#       number of overrides (int)
#           hashid (int)
#           type (int) (1: float, 2: bool ... )
#           value (variant, 4 bytes)

def strhash(str):
    hash = 0
    for c in str:
        v = ord(c.lower())
        hash = (v + (hash << 6) + (hash << 16) - hash) & 0xFFFFFFFFL
    return hash

def WriteComponent(cmpnode, outfile, endian):
    cmpname = cmpnode.attributes["type"].value.encode('ascii')
    vals = cmpnode.getElementsByTagName("val")
    outfile.write(struct.pack(endian + 'II', strhash(cmpname), len(vals)))
    for val in vals:
        name = val.attributes["name"].value.encode('ascii')
        type = val.attributes["type"].value.encode('ascii')
        val = val.attributes["val"].value.encode('ascii')
        outfile.write(struct.pack(endian + 'I', strhash(name)))

        if type=="num":
			try:
				outfile.write(struct.pack(endian + 'If', 0, float(val)))
			except:
				print( cmpnode )
				print( name, type, val )
				raise
        elif type=="bool":
            outfile.write(struct.pack(endian + 'I?', 1, bool(val)))
        elif type=="hash":
            outfile.write(struct.pack(endian + 'II', 2, strhash(val)))
        else:
            assert False, "BAD TYPE " + type

def CompileLevel(infile, outfilename, endianstring):
    doc = xml.dom.minidom.parse(infile)
    outfile = open(outfilename, 'wb')
    # write out the header
    outfile.write(struct.pack(endianstring +  'cccci', 'L', 'E', 'V', 'L', VERSION))

    scenarioname = doc.childNodes[0].attributes["scenario"].value.encode('ascii')
    outfile.write(struct.pack(endianstring + 'i' + str(len(scenarioname)) + 's', len(scenarioname), scenarioname))

    # now handle entities
    entities = doc.getElementsByTagName("entity")
    outfile.write(struct.pack(endianstring + 'i', len(entities)))
    for ent in entities:

        prefabname = ent.attributes["prefab"].value.encode('ascii')
        outfile.write(struct.pack(endianstring + 'i' + str(len(prefabname)) + 's', len(prefabname), prefabname))

        name = ent.attributes["name"].value.encode('ascii')
        outfile.write(struct.pack(endianstring + 'i' + str(len(name)) + 's', len(name), name))

        tags = string.split(ent.attributes["tags"].value.encode('ascii'))
        outfile.write(struct.pack(endianstring + 'i', len(tags)))
        for tag in tags:
            outfile.write(struct.pack(endianstring + 'I', strhash(tag)))


        components = ent.getElementsByTagName("cmp")
        outfile.write(struct.pack(endianstring + 'i', len(components)))
        for component in components:
            WriteComponent(component, outfile, endianstring)

    outfile.close()

def main(args):

    parser = argparse.ArgumentParser(description='Compile a level xml to platform-specific binary.')
    parser.add_argument('infile', action="store")
    parser.add_argument('outfile', action="store")
    parser.add_argument('--bigendian', action="store_true", default=False)

    results = parser.parse_args()
    endian = "<"
    if results.bigendian:
        endian = ">"
    CompileLevel(results.infile, results.outfile, endian)


if __name__ == "__main__":
    main(sys.argv[1:])
    print( "Successfully exported", sys.argv[1], "to", sys.argv[2] )
