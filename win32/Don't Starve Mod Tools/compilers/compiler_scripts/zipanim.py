import sys
import os
import zipfile

zip_path = sys.argv[1]
build_path = sys.argv[2]
anim_path = sys.argv[3]
image_list_path = sys.argv[4]

zip = zipfile.ZipFile(zip_path, 'w')
zip.write(build_path, "build.xml")
zip.write(anim_path, "animation.xml")

for img in open(image_list_path, "r").readlines():
	path = img[:-1]
	zip.write(path, os.path.basename(path))
		
