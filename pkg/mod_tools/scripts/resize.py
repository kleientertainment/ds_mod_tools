import sys
import os
from PIL import Image

source_folder = sys.argv[1]
dest_folder = sys.argv[2]

sizes = {}
for root, dirs, files in os.walk(source_folder):
    for f in files:        
        if f[-3:] == 'png':
        	path = root + '\\' + f
        	im=Image.open(path)
        	sizes[f] = im.size

for root, dirs, files in os.walk(dest_folder):
    for f in files:        
        if f[-3:] == 'png':
        	if f in sizes:
        		new_size = sizes[f]
        		path = root + '\\' + f
        		im=Image.open(path)
        		im=im.resize(new_size)
        		im.save(path)
