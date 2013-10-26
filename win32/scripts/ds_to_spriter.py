import xml.etree.ElementTree as et
from xml.dom import minidom
from PIL import Image
import sys
import os
import shutil
import math
def sign(a):
	if a > 0:
		return 1
	return -1
tree = et.parse(sys.argv[1])
build = tree.getroot()

doc = et.Element('spriter_data')
doc.set('scml_version', '1.0')
doc.set('generator', 'BrashMonkey Spriter')
doc.set('generator_version', 'b5')

folders = {}

scale_f = 0.33333
folder_id = 0
for symbol in build:
	folder = et.SubElement(doc, 'folder')
	folder.set('id', str(folder_id))	
	folder.set('name', symbol.get('name'))	
	file_id = 0
	files = {}

	if not os.path.exists(symbol.get('name')):
		os.makedirs(symbol.get('name'))

	for frame in symbol:		
		image_name = frame.get('image')+'.png'
		source_path = os.path.split(sys.argv[1])[0] + '//' + image_name 
		image_path = os.path.split(sys.argv[1])[0] + '//' + symbol.get('name') + '/' + image_name
		x = float(frame.get('x'))
		y = float(frame.get('y'))
		w = float(frame.get('w'))
		h = float(frame.get('h'))
		if os.path.exists(source_path):
			if not os.path.exists(os.path.split(image_path)[0]):
				os.makedirs(os.path.split(image_path)[0])
			im=Image.open(source_path)
			w = im.size[0] * scale_f
			h = im.size[1] * scale_f
			im = im.resize((int(w), int(h)))
			im.save(image_path)
		pivot_x = str(0.5 - x/w*scale_f)
		pivot_y = str(0.5 + y/h*scale_f)		
		file = et.SubElement(folder, 'file')
		file.set('id', str(file_id))
		file.set('name', image_path[image_path.find('/')+2:])
		file.set('width', str(int(w)))
		file.set('height', str(int(h)))
		file.set('pivot_x', pivot_x)
		file.set('pivot_y', pivot_y)
		files[str(file_id)] = file
		file_id = file_id + 1
	folders[symbol.get('name')] = (str(folder_id), files)
	folder_id = folder_id + 1

entity = et.SubElement(doc, 'entity')
entity.set('id', '0')
entity.set('name', 'template')
animation = et.SubElement(entity, 'animation')
animation.set('id', '0')
animation.set('name', 'BUILD_PLAYER')
animation.set('length', '1000')
mainline = et.SubElement(animation, 'mainline')
main_key = et.SubElement(mainline, 'key')

main_key.set('id', '0')

anim_setups = {'idle_down': [], 'idle_side': [], 'idle_up': []}
anims = et.parse(sys.argv[2]).getroot()
for anim in anims:
	if anim.get('name') in anim_setups:
		frame = anim[0]
		setup = anim_setups[anim.get('name')]
		for ele in frame:
			setup.append(ele)

timeline_id = 0
for name, setup in anim_setups.items():
	if len(setup) > 0:
		for ele in setup:
			if ele.get('name') in folders: 
				folder = folders[ele.get('name')]
				folder_id = folder[0]
				files = folder[1]
				if ele.get('frame') in files:
					frame = files[ele.get('frame')]
					object_ref = et.SubElement(main_key, 'object_ref')
					object_ref.set('id', str(timeline_id))
					object_ref.set('name', ele.get('name'))
					object_ref.set('folder', folder_id)
					object_ref.set('file', ele.get('frame'))
					object_ref.set('abs_x', '0')
					object_ref.set('abs_y', '0')
					object_ref.set('abs_pivot_x', frame.get('pivot_x'))
					object_ref.set('abs_pivot_y', frame.get('pivot_y'))
					object_ref.set('abs_angle', '360')
					object_ref.set('abs_scale_x', '1')
					object_ref.set('abs_scale_y', '1')
					object_ref.set('abs_a', '1')
					object_ref.set('timeline', str(timeline_id))
					object_ref.set('key', '0')
					object_ref.set('z_index', str(200 - int(ele.get('depth'))))
					timeline_id = timeline_id + 1		

setup_offset = -250
timeline_id = 0
for name, setup in anim_setups.items():
	if len(setup) > 0:
		for ele in setup:	
			if ele.get('name') in folders:
				folder = folders[ele.get('name')]
				folder_id = folder[0]
				files = folder[1]		
				if ele.get('frame') in files:		
					x = str(float(ele.get('m_tx'))*scale_f + setup_offset)
					y = str(-float(ele.get('m_ty'))*scale_f)
					a = float(ele.get('m_a'))
					b = float(ele.get('m_b'))
					c = float(ele.get('m_c'))
					d = float(ele.get('m_d'))
					rads = math.atan(c/d)					
					angle = str(rads*360.0/(2.0*3.14159))
					scale_x = str(sign(a)*math.sqrt(a*a+b*b))
					scale_y = str(sign(d)*math.sqrt(c*c+d*d))
					timeline = et.SubElement(animation, 'timeline')
					timeline.set('id', str(timeline_id))
					timeline.set('name', ele.get('name')+'_'+ele.get('frame')+'_'+str(timeline_id))
					key = et.SubElement(timeline, 'key')
					key.set('id', '0')
					key.set('spin', '0')
					object_ref = et.SubElement(key, 'object')
					object_ref.set('id', '0')
					object_ref.set('folder', folder_id)
					object_ref.set('file', ele.get('frame'))
					object_ref.set('x', x)
					object_ref.set('y', y)
					object_ref.set('scale_x', scale_x)
					object_ref.set('scale_y', scale_y)					
					object_ref.set('angle', angle)	
					timeline_id = timeline_id + 1
	setup_offset += 250

"""
ref_id = 0
folder_id = 0
for symbol in build:	
	file_id = 0
	for frame in symbol:		
		object_ref = et.SubElement(main_key, 'object_ref')
		object_ref.set('id', str(ref_id))
		object_ref.set('name', frame.get('image'))
		object_ref.set('folder', str(folder_id))
		object_ref.set('file', str(file_id))
		object_ref.set('abs_x', '0')
		object_ref.set('abs_y', '0')
		object_ref.set('abs_pivot_x', frame.get('x'))
		object_ref.set('abs_pivot_y', frame.get('y'))
		object_ref.set('abs_angle', '360')
		object_ref.set('abs_scale_x', '1')
		object_ref.set('abs_scale_y', '1')
		object_ref.set('abs_a', '1')
		object_ref.set('timeline', str(ref_id))
		object_ref.set('key', '0')
		object_ref.set('z_index', '0')
		ref_id = ref_id + 1
		file_id = file_id + 1
	folder_id = folder_id + 1

timeline_id = 0
ref_id = 0
folder_id = 0
for symbol in build:	
	file_id = 0
	for frame in symbol:		
		timeline = et.SubElement(animation, 'timeline')
		timeline.set('id', str(timeline_id))
		timeline.set('name', frame.get('image'))
		key = et.SubElement(timeline, 'key')
		key.set('id', '0')
		key.set('spin', '0')
		object_ref = et.SubElement(key, 'object')
		object_ref.set('id', '0')
		object_ref.set('folder', str(folder_id))
		object_ref.set('file', str(file_id))
		object_ref.set('x', '0')
		object_ref.set('y', '0')
		timeline_id = timeline_id + 1
		file_id = file_id + 1
	folder_id = folder_id + 1
"""
f = open(sys.argv[3], 'w')
f.write(minidom.parseString(et.tostring(doc)).toprettyxml())
