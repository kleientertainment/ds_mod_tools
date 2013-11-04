import sys, os, tempfile, argparse
import ModelCompiler
from Vector import *
from Colour import Colour

WorkingDir = None

def ParseFace( face, face_data ):
	def ComponentGenerator( components ):
		for component in components:
			yield component

	for vertex in face_data.split( " " )[ 1: ]:
		components = vertex.split( "/" )
		components = ComponentGenerator( components )

		# According to the waveform obj file format, you can have a vertex
		# that looks like 1//1 if the vertex has no texcoords, so we need
		# to handle that case by doing extra validation

		# Also note that the obj file format uses 1 based indices, but python is 0 based, hence
		# the -1s below.
		pos_index = -1
		uv_index = -1
		normal_index = -1

		pos_index = int( components.next() ) - 1

		index = components.next()
		if index != "":
			uv_index = int( index ) - 1

		index = components.next()
		if index != "":
			normal_index = int( index ) - 1

		# If any of these are true then we really want to output different vertex buffer formats
		assert pos_index != -1
		assert uv_index != -1
		assert normal_index != -1

		vtx = ModelCompiler.Vertex( pos_index, uv_index, normal_index )
		face.Vertices += [ vtx ]

	assert len( face.Vertices ) == 3 # otherwise we have a non-triangular face and more code needs to be implemented

def ParseMesh( mesh, material_name, face_data, model_vertices ):
	mesh.MaterialName = material_name
	for data in face_data:
		if data.startswith( "f " ):
			face = ModelCompiler.Face()
			ParseFace( face, data )
			mesh.Faces += [ face ]

def ParseModel( model, filename ):
	# The GenericGenerator is a helper function used to parse data from an obj file
	# It takes a list of strings, a prefix to search for an an obj type to covert to, then
	# whenever the prefix is encoutered it will generate the obj and return it to you
	# through an iterator interface
	def GenericGenerator( string_collection, prefix, objtype ):
		prefix_len = len( prefix )
		for item in string_collection:
			item = item.strip()
			if item.startswith( prefix ):
				yield objtype( item[ prefix_len : ].split( " " ) )

	global WorkingDir
	WorkingDir = os.path.dirname( filename )

	active_mesh = None
	with open( filename, "rt" ) as f:
		lines = f.readlines()

		# This iterates through the data several times so it could be readily
		# sped up by iterating just once. The change would make for uglier code though.
		
		# Read all the raw data from the file (positions, uvs, normals)
		model.Positions = [ Vector3( [ v.x, v.y, v.z ] ) for v in GenericGenerator( lines, "v ", ModelCompiler.Position ) ]
		model.UVs = [ v for v in GenericGenerator( lines, "vt ", ModelCompiler.UV ) ]
		model.Normals = [ v for v in GenericGenerator( lines, "vn ", ModelCompiler.Normal ) ]

		# Now iterate through the file looking for the structures that use the raw
		# information built in the previous step 
		for line_idx in range( len( lines ) ):
			line = lines[ line_idx ].strip()
			if line == "" or line[0] == "#":
				continue

			# If we find a usemtl line, find all the faces following that line and
			# make a mesh out of them.
			if line.startswith( "usemtl " ):
				mesh_end_idx = line_idx + 1
				while mesh_end_idx < len( lines ) and not lines[ mesh_end_idx ].startswith( "g " ):
					mesh_end_idx += 1

				mesh = ModelCompiler.Mesh()
				ParseMesh( mesh, line.split( " " )[ 1 ], lines[ line_idx + 1 : mesh_end_idx ], model.Vertices )
				model.Meshes += [ mesh ]
				line_idx = mesh_end_idx
			elif line.startswith( "mtllib " ):
				materiallib = ModelCompiler.MaterialLib()
				ParseMaterialLib( materiallib, os.path.join( WorkingDir, line.split( " " )[ 1 ] ) )
				model.MaterialLib = materiallib

def ParseMaterialLib( materiallib, filename ):
	materiallib.Materials = {}
	material_start_idx = -1

	materiallib.Name = os.path.basename( filename )

	with open( filename, "rt" ) as f:
		lines = f.readlines()
		for line_idx in range( len( lines ) ):
			line = lines[ line_idx ]

			if line_idx == len( lines ) - 1 and material_start_idx != -1:
				material_lines = lines[ material_start_idx : line_idx + 1 ]
				material = ModelCompiler.Material()
				ParseMaterial( material, material_lines )
				materiallib.Materials[ material.Name ] = material

			if line.startswith( "newmtl" ):
				if material_start_idx != -1:
					material_lines = lines[ material_start_idx : line_idx ]
					material = ModelCompiler.Material()
					ParseMaterial( material, material_lines )
					materiallib.Materials[ material.Name ] = material
				material_start_idx = line_idx

def ParseMaterial( material, material_lines ):
	material.Name = material_lines[0].strip().split( " " )[ 1 ]
	material.TextureFilenames = {}
	material.Diffuse = Colour( 1.0, 1.0, 1.0, 1.0 )

	for line in material_lines[ 1: ]:
		"""map_Kd -options args filename 

		Specifies that a color texture file or color procedural texture file is linked to the diffuse reflectivity of the material. During rendering, the map_Kd value is multiplied by the Kd value.
		"filename" is the name of a color texture file (.mpc), a color procedural texture file (.cxc), or an image file. 
		The options for the map_Kd statement are listed below. These options are described in detail in "Options for texture map statements" on page 5-18. 

		-blendu on | off 
		-blendv on | off 
		-cc on | off 
		-clamp on | off 
		-mm base gain 
		-o u v w 
		-s u v w  # Scale
		-t u v w 
		-texres value 
		"""	
		if line.startswith( "map_Kd" ):
			args = line.split( " " )[ 1: ]
			filename = args[ -1 ].strip()
			temp_dir = tempfile.gettempdir()
			src_filename = os.path.join( WorkingDir, filename )

			prefix, ext = os.path.splitext( filename )
			tex_filename = prefix + ".tex"
			dest_filename = os.path.join( temp_dir, tex_filename )

			material.TextureFilenames[ filename ] = ( src_filename, dest_filename )
		elif line.startswith( "Kd" ):
			args = line.split( " " )[ 1: ]
			material.Diffuse = Colour( float( args[0] ), float( args[1] ), float( args[2] ), 1.0 )


if __name__ == "__main__":
	parser = argparse.ArgumentParser( description = "Wavefront OBJ file compiler" )
	parser.add_argument( "-i", "--infile", action="store", required=True )
	parser.add_argument( "-o", "--outfile", action="store" )
	parser.add_argument( "-g", "--gencollision", action="store_true" )
	args = parser.parse_args()

	model = ModelCompiler.Model()

	if args.gencollision:
		model.GenerateCollisionData = True

	ParseModel( model, args.infile )

	if args.outfile == None:
		zf, ext = os.path.splitext( args.infile )
		args.outfile = zf + ".zip"

	model.Compile( args.outfile )

