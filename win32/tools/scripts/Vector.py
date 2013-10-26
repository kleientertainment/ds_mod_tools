
class Vector2(object):
	def __init__( self, args ):
		x, y = args
		self.x, self.y = float( x ), float( y )

	def __str__( self ):
		s = "("
		s += ", ".join( [ str( i ) for i in [ self.x, self.y ] ] )
		s += ")"

		return s

class Vector3(object):
	def __init__( self, args ):
		x, y, z = args
		self.x, self.y, self.z = float( x ), float( y ), float( z )

	def __str__( self ):
		s = "("
		s += ", ".join( [ str( i ) for i in [ self.x, self.y, self.z ] ] )

		s += ")"
		return s

	def Min( self, v ):
		return Vector3( [ min( self.x, v.x ), min( self.y, v.y ), min( self.z, v.z ) ] )

	def Max( self, v ):
		return Vector3( [ max( self.x, v.x ), max( self.y, v.y ), max( self.z, v.z ) ] )

