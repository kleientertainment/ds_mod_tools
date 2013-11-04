import sys, os, re, tempfile
import xml
from xml.etree.ElementTree import ElementTree
from Vector import *
import ModelCompiler
import Units

Polygons = []

NameSpaces = {}

LayerTextureMap = {
		'water' : [ 'Ground_water.png' ],
		}

def parse_nsmap(file):
	events = "start", "start-ns", "end-ns"

	root = None
	ns_map = []

	for event, elem in xml.etree.ElementTree.iterparse(file, events):
		if event == "start-ns":
			ns_map.append(elem)
			NameSpaces[ elem[0] ] = elem[1]
		elif event == "end-ns":
			ns_map.pop()
		elif event == "start":
			if root is None:
				root = elem

	return ElementTree(root)


def RemoveNameSpace( string ):
	return string[ string.find( "{" ) : string.find( "}" ) + 1 ]

def Parse( filename ):
	path = os.path.dirname( filename )

	tree = ElementTree()
	tree.parse( filename )

	root = tree.getroot()
	prefix = RemoveNameSpace( root.tag )

	groups = root.findall( prefix + "g" )

	model = ModelCompiler.Model()
	mesh_map = {}

	model.MaterialLib = ModelCompiler.MaterialLib()
	model.MaterialLib.Name = "svgmaterials"

	for group in groups:
		polygons = group.findall( prefix + "polygon" )

		label = group.attrib[ '{' + NameSpaces[ u'inkscape' ] + '}label' ]

		for polygon in polygons:
			points = polygon.attrib[ "points" ].strip()
			points = re.sub( ' +', ' ', points ) # replace multiple spaces with a single space
			points = points.split( " " )

			positions = model.Positions
			normals = model.Normals
			uvs = model.UVs

			mesh = None
			if not label in mesh_map:
				mesh = ModelCompiler.Mesh()
				model.Meshes += [ mesh ]
				mesh_map[ label ] = mesh

				material = ModelCompiler.Material()
				material.Name = label

				model.MaterialLib.Materials[ label ] = material

				textures = LayerTextureMap[ label ]
				for texture in textures:
					src_filename = os.path.join( path, texture )

					temp_dir = tempfile.gettempdir()
					prefix, ext = os.path.splitext( texture )
					tex_filename = prefix + ".tex"
					dest_filename = os.path.join( temp_dir, tex_filename )

					material.TextureFilenames[ texture ] = ( src_filename, dest_filename )
			else:
				mesh = mesh_map[ label ]

			# TODO: Map the material name from the label
			mesh.MaterialName = label

			face = ModelCompiler.Face()

			mesh.MinIndex = len( model.Positions )
			mesh.MaxIndex = mesh.MinIndex + len( points ) - 1
			mesh.Faces += [ face ]

			mesh_min_pos = Vector3( [ sys.float_info.max, sys.float_info.max, sys.float_info.max ] )
			mesh_max_pos = Vector3( [ -sys.float_info.max, -sys.float_info.max, -sys.float_info.max ] )

			for point in points:
				cur_len = len( model.Positions )
				vtx = ModelCompiler.Vertex( cur_len, cur_len, cur_len )
				model.Vertices[ vtx ] = True

				x, z = [ float( val ) for val in point.split( "," ) ] # split and convert to floats
				p = Vector3( [ x, 0, z ] )
				positions += [ p ]
				normals += [ Vector3( [ 0, 1, 0 ] ) ]

				model.MinPosition = model.MinPosition.Min( p )
				model.MaxPosition = model.MaxPosition.Max( p )
				
				mesh_min_pos = mesh_min_pos.Min( p )
				mesh_max_pos = mesh_max_pos.Max( p )

				face.PositionIndices += [ cur_len ]
				face.NormalIndices += [ cur_len ]
				face.UVIndices += [ cur_len ]

				mesh.VertexIndices += [ cur_len ]

			# The positions are all specified in world space, which is in meters.
			# To generate uvs we need to take the size of the texture that should
			# be applied to this object, convert it to meters and then use that to
			# determine the uv of a given vertex
			for pos in positions:
				u, v = pos.x, pos.z

				u -= mesh_min_pos.x
				v -= mesh_min_pos.z

				u /= Units.PixelsPerMeter
				v /= ( mesh_max_pos.z - mesh_min_pos.z )

				uv = ModelCompiler.Vector2( [ u, v ] )
				uvs += [ uv ]
	return model

				
if __name__ == "__main__":
	src_filename = sys.argv[1]
	dest_filename = sys.argv[2]

	parse_nsmap( open( src_filename, "rt" ) )

	model = Parse( src_filename )
	model.Compile( dest_filename )
