import collections, os, sys, argparse, struct

ProfileRegion = collections.namedtuple( "ProfileRegion", "name, indentation" )

ProfileRegions = []

ProfileEntry = collections.namedtuple( "ProfileEntry", "totalCalls, timeTaken, minTime, maxTime" )
format_str = "{0:30s}  {1:<16} {2: =12.3f} {3: =12.3f} {4: =12.3f}"

def PrintFrame( frame ):
	print( "{0:30s}  {1:<16} {2:>12} {3:>12} {4:>12}".format( "Name", "# Calls", "Total (ms)", "Min (ms)", "Max (ms)" ) )
	print( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" )

	for idx, entry in frame:
		indent = ""
		for i in range( ProfileRegions[ idx ].indentation ):
			indent += " "
		print( format_str.format( indent + ProfileRegions[ idx ].name, entry.totalCalls, entry.timeTaken * 1000, entry.minTime * 1000, entry.maxTime * 1000) )

num_regions = len( ProfileRegions )
if __name__ == "__main__":
	parser = argparse.ArgumentParser( description = "Profile data parser" )
	parser.add_argument( 'infile' )

	args = parser.parse_args()

	count_struct = struct.Struct( 'I' )
	entry_struct = struct.Struct( 'Ifff' )

	count_size = count_struct.size
	entry_size = entry_struct.size

	frames = []

	with file( args.infile, "rb" ) as f:
		data = f.read()

		data_offset = 0

		num_entries = count_struct.unpack_from( data )[0]
		data_offset += count_struct.size
		print( "Deserializing " + str( num_entries ) + " entries per frame" )

		for i in range( num_entries ):
			strlen = count_struct.unpack_from( data, data_offset )[ 0 ]
			data_offset += count_struct.size

			strfmt = "{0}s".format( strlen )
			name = struct.unpack_from( strfmt, data, data_offset )[ 0 ]
			data_offset += strlen

			indent = count_struct.unpack_from( data, data_offset )[ 0 ]
			data_offset += count_struct.size

			ProfileRegions.append( ProfileRegion( name, indent ) )

		data_len = len( data )

		frame_idx = 0
		while( data_offset < data_len ):
			frame_data = []

			#print( "Frame {0} has offset {1:x}".format( frame_idx, data_offset ) )
			frame_idx += 1

			try:
				for i in range( num_entries ):
					totalCalls, timeTaken, minTime, maxTime = entry_struct.unpack_from( data, data_offset )
					data_offset += entry_struct.size

					entry = ProfileEntry( totalCalls, timeTaken, minTime, maxTime )

					#print( "\t" + ", ".join( ( str( entry.totalCalls ), str( entry.timeTaken ) , str( entry.minTime ), str( entry.maxTime ) ) ) )

					frame_data.append( [ i, entry ] )

				frames.append( frame_data )
			except:
				print( "Processed {0} frames".format( len( frames ) ) )
				break

			#if frame_idx > 5:
			#	sys.exit( 0 )

	if len( frames ) > 0:
		data = None
		frame_idx = 0
		last_cmd = None
		while data != "exit" and data != "quit":
			print
			print( "Num Frames: {0}".format( len( frames ) ) )
			print( "Cur Frame:  {0}".format( frame_idx ) )
			print

			PrintFrame( frames[ frame_idx ] )
			
			data = raw_input(":")
			try:
				frame_idx = int( data )
			except:
				pass

			if data == "":
				data = last_cmd
			else:
				last_cmd = data

			if data == "+":
				frame_idx += 1
			elif data == "-":
				frame_idx -= 1

			frame_idx = min( frame_idx, len( frames ) -1 )
			frame_idx = max( 0, frame_idx )

