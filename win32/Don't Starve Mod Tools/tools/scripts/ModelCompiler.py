import sys, os, struct, tempfile, zipfile, StringIO
from PIL import Image
from collections import namedtuple, OrderedDict
from Vector import *
import Endian, textureconverter

EndianString = None
IntFormatter = None
UIntFormatter = None
UShortFormatter = None
FloatFormatter = None
Vector2Formatter = None
Vector3Formatter = None

INVERT_Z_COMPONENT = True

def SetEndian( endian ):
	global EndianString
	global IntFormatter
	global UIntFormatter
	global UShortFormatter
	global FloatFormatter
	global Vector2Formatter
	global Vector3Formatter
	EndianString = endian
	IntFormatter = struct.Struct( EndianString + "i" )
	UIntFormatter = struct.Struct( EndianString + "I" )
	UShortFormatter = struct.Struct( EndianString + "H" )
	FloatFormatter = struct.Struct( EndianString + "f" )
	Vector2Formatter = struct.Struct( EndianString + "ff" )
	Vector3Formatter = struct.Struct( EndianString + "fff" )

SetEndian( Endian.Little )

Vertex = namedtuple( "Vertex", "Position, UV, Normal" )

def WriteString( stream, string ):
	packed_data = struct.pack( EndianString + "I" + str( len( string ) ) + "s", len( string ), string )
	stream.write( packed_data )

def WriteUInt( stream, val ):
	stream.write( UIntFormatter.pack( val ) )

def WriteUShort( stream, val ):
	stream.write( UShortFormatter.pack( val ) )

def WriteVector2( stream, v ):
	global Vector2Formatter
	stream.write( Vector2Formatter.pack( v.x, v.y ) )
	
def WriteVector3( stream, v ):
	global Vector3Formatter
	stream.write( Vector3Formatter.pack( v.x, v.y, v.z ) )

def WriteFloat( stream, val ):
	global FloatFormatter
	stream.write( FloatFormatter.pack( val ) )
		
class Position( Vector3 ):
	def __init__( self, args ):
		super( Position, self ).__init__( args )

	def __str__( self ):
		return super( Position, self ).__str__()

class Normal( Vector3 ):
	def __init__( self, args ):
		super( Normal, self ).__init__( args )

class UV( Vector2 ):
	def __init__( self, args ):
		super( UV, self ).__init__( args )

class Face:
	"A face is composed of a set of vertices which are a combination of pos/uv/normal indices into the global pos/uv/normal tables"

	def __init__( self ):
		self.Vertices = []

	def __str__( self ):
		s = "Face: NumVertices( {0} )".format( len( self.Vertices ) ) + "\n\t\t".join( [ "" ] + [ str( v ) for v in self.Vertices ] )
		return s

	def Reverse( self ):
		self.Vertices.reverse()
	

class Mesh:
	"Meshes are simply index buffers that reference the raw vertex data in the parent model."
	def __init__( self ):
		self.MinIndex = sys.maxint
		self.MaxIndex = -1
		self.VertexIndices = []
		self.Faces = []

	def __str__( self ):
		s = "Mesh - Material( {0} ), Faces( {1} )".format( self.MaterialName, len( self.Faces ) )
		s += "\n\t".join( [ "" ] + [ str( face ) for face in self.Faces ] )
		return s

	def Compile( self, stream, model_vertices, materials ):
		# TODO: Split all the faces into triangles...

		# Build a list of unique vertices
		for face in self.Faces:
			# We assume triangles at this time
			assert len( face.Vertices ) == 3

			face_vertices = []
			for vtx in face.Vertices:
				if vtx not in model_vertices:
					model_vertices[ vtx ] = True

				# TODO: Figure out if this should be a binary search. If it's linear it's probably quite slow
				vtx_idx = model_vertices.keys().index( vtx ) 
				face_vertices += [ vtx_idx ]

				self.MinIndex = min( self.MinIndex, vtx_idx )
				self.MaxIndex = max( self.MaxIndex, vtx_idx )

			if INVERT_Z_COMPONENT:
				face_vertices.reverse()

			self.VertexIndices += face_vertices

		# TODO: Even with different indices, it's still technically possible
		# for there to be duplicate vertices though it is unlikely. To be completely
		# "correct" you would do a post pass to check for dupes based on values

		WriteUInt( stream, materials.index( self.MaterialName ) )

		#print( len( self.VertexIndices ), self.MinIndex, self.MaxIndex )
		WriteUShort( stream, len( self.VertexIndices ) )
		WriteUShort( stream, self.MinIndex )
		WriteUShort( stream, self.MaxIndex )
		assert len( self.VertexIndices ) < 65536 # using shorts for indices...
		for vtx_idx in self.VertexIndices:
			WriteUShort( stream, vtx_idx )

class Model:
	def __init__( self ):
		self.MinPosition = Vector3( [ sys.float_info.max, sys.float_info.max, sys.float_info.max ] )
		self.MaxPosition = Vector3( [ -sys.float_info.max, -sys.float_info.max, -sys.float_info.max ] )
		self.Positions = []
		self.UVs = []
		self.Normals = []
		self.Meshes = []
		self.Vertices = OrderedDict()
		self.MaterialLib = None
		self.GenerateCollisionData = False

	def Compile( self, filename ):
		if INVERT_Z_COMPONENT:
			for i in range( len( self.Positions ) ):
				self.Positions[ i ].z = - self.Positions[ i ].z
				
		# AABB calculation
		for pos in self.Positions:
			self.MinPosition = self.MinPosition.Min( Vector3( [ pos.x, pos.y, pos.z ] ) )
			self.MaxPosition = self.MaxPosition.Max( Vector3( [ pos.x, pos.y, pos.z ] ) )

		with zipfile.ZipFile( filename, "w", zipfile.ZIP_DEFLATED ) as zf:
			# TODO: Write out the vertex description so it's not hard coded in the game

			# Be very careful re-ordering anything below! Many of these Compile calls have
			# side effects.

			model = StringIO.StringIO();
			WriteVector3( model, self.MinPosition )
			WriteVector3( model, self.MaxPosition )

			# Write out each of the meshes
			meshes = StringIO.StringIO()
			WriteUInt( meshes, len( self.Meshes ) )
			for mesh in self.Meshes:
				mesh.Compile( meshes, self.Vertices, self.MaterialLib.Materials.keys() )
			zf.writestr( "__meshes__", meshes.getvalue() )

			# Write out all the vertex data, interleaved
			WriteUInt( model, len( self.Vertices ) )
			for vtx in self.Vertices.keys():
				pos = self.Positions[ vtx.Position ]
				uv = self.UVs[ vtx.UV ]
				normal = self.Normals[ vtx.Normal ]

				WriteVector3( model, pos )
				WriteVector2( model, uv )
				WriteVector3( model, normal )
			zf.writestr( "__model__", model.getvalue() )

			self.MaterialLib.Compile( zf )

			collision = StringIO.StringIO()
			if self.GenerateCollisionData:
				WriteUInt( collision, len( self.Vertices ) )
				for vtx in self.Vertices.keys():
					pos = self.Positions[ vtx.Position ]
					WriteVector3( collision, pos )
				zf.writestr( "__collision__", collision.getvalue() )
			

class MaterialLib:
	def __init__( self ):
		self.Name = None
		self.Materials = {}

	def Compile( self, container_file ):
		stream = StringIO.StringIO()

		WriteUInt( stream, len( self.Materials ) )
		for material_name in self.Materials:
			self.Materials[ material_name ].Compile( stream )
		container_file.writestr( "__materials__", stream.getvalue() )

		for material in self.Materials.values():
			for filename, tex_data in material.Textures.items():
				container_file.writestr( os.path.basename( filename ), tex_data )

class Material:
	"""	To properly populate a material you want to fill out the TextureFilenames map with a tuple of ( src_filename, dest_filename).
		Then, when compile is called the texture converter will be called and everything will be properly baked/zipped.

		filename is the name of the file to be used in the zip (i.e. does not contain a path)
		src_filename is the relative or absolute path to the PNG/source texture
		dest_filename should be the path & filename to a temporary file to be used for the texture conversion process. This file WILL be removed"""

	def __init__( self ):
		self.Name = None
		self.TextureFilenames = {}

	def Compile( self, stream ):
		self.Textures = {} 

		for tex_name, filenames in self.TextureFilenames.items():
			src_filename, dest_filename = filenames

			img = Image.open( src_filename )
			pixel_format = "bc3"
			#if img.mode == "RGB":
			#	pixel_format = "bc1"

			# TODO: Output to an intermediate directory and do timestamp checking
			print( "Converting " + src_filename )
			try:
				textureconverter.MipAndConvert( src_filename, dest_filename, pixel_format )

				with open( dest_filename, "rb" ) as f:
					tex_data = f.read()
					self.Textures[ dest_filename ] = tex_data
			finally:
				if os.path.exists( dest_filename ):
					os.remove( dest_filename )

		WriteString( stream, self.Name )

		if len( self.TextureFilenames.items() ) == 0:
			WriteString( stream, "data/shaders/model_coloured.ksh" )
		else:
			WriteString( stream, "data/shaders/model_textured.ksh" )

		WriteFloat( stream, self.Diffuse.r )
		WriteFloat( stream, self.Diffuse.g )
		WriteFloat( stream, self.Diffuse.b )
		WriteFloat( stream, self.Diffuse.a )

		WriteUInt( stream, len( self.Textures ) )
		for filename in self.Textures.keys():
			WriteString( stream, os.path.basename( filename ) )

