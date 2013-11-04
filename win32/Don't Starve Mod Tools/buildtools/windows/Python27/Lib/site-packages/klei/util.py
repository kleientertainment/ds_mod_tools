import os, subprocess, tempfile, shutil

def Execute( cmd, display_cmd=True ):
    if display_cmd:
        print( cmd )
    result = os.system( '"' + cmd + '"' )
    return result

def IsFileNewer( src_path, dest_path ):
    dest_path = os.path.normpath( dest_path )
    src_path = os.path.normpath( src_path )
    src_time = os.stat( src_path ).st_mtime
    for root, dirs, files in os.walk( src_path ):
        for filename in files:
            src_time = max( os.stat( os.path.join( root, filename ) ).st_mtime, src_time )
    dest_time = None if not os.path.exists( dest_path ) else os.stat( dest_path ).st_mtime

    return dest_time == None or src_time > dest_time

def IsAnyFileNewer( src_paths, dest_paths ):
    newer = False

    if type( src_paths ) != list:
        src_paths = [ src_paths ]

    if type( dest_paths ) != list:
        dest_paths = [ dest_paths ]

    for src_path in src_paths:
        for dest_path in dest_paths:
            newer = newer or IsFileNewer( src_path, dest_path )

    return newer

class TemporaryDirectory:
    def __enter__( self ):
        self.dirname = tempfile.mkdtemp()
        #self.dirname = r"c:\temp\dstmp"
        return self.dirname

    def __exit__( self, err_t, value, traceback ):
        shutil.rmtree( self.dirname )

    def __str__( self ):
        return self.dirname

