import sys
import os
import zipfile

output_folder = sys.argv[1]
zip_path = sys.argv[2]
build_path = os.path.join( output_folder, 'build.xml' )
anim_path = os.path.join( output_folder, 'animation.xml' )
image_list_path = os.path.join( output_folder, 'images.lst' )

zip = zipfile.ZipFile(zip_path, 'w')
zip.write(build_path, "build.xml")
zip.write(anim_path, "animation.xml")

for img in open(image_list_path, "r").readlines():
	path = img[:-1]
	zip.write(path, os.path.basename(path))
		
