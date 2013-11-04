import argparse
from collections import namedtuple
import Image, ImageDraw
import sys

BBox = namedtuple("Bbox", 'x y w h')
Regions = namedtuple( "Regions", "alpha, opaque" )

class BlockType:
	OPAQUE = 1
	EMPTY = 2
	ALPHA = 3
	UNKNOWN = 4

def BlockName(block):
	if block == BlockType.OPAQUE:
		return "Opaque"
	elif block == BlockType.EMPTY:
		return "Empty"
	elif block == BlockType.ALPHA:
		return "Alpha"
	return "Unknown"


def Analyze(im, bbox):
	numalpha = 0
	numblank = 0
	numopaque = 0

	pixels = im.load()
	for y in range(bbox.y, bbox.y + bbox.h):
		for x in range(bbox.x, bbox.x + bbox.w):
			p = pixels[x,y]
			if p[3] == 0:
				numblank += 1
			elif p[3] == 255:
				numopaque += 1
			else:
				numalpha += 1

	if numalpha == 0 and numblank == 0:
		return BlockType.OPAQUE
	elif numalpha == 0 and numopaque == 0:
		return BlockType.EMPTY
	else:
		return BlockType.ALPHA


class QuadTreeNode:

	def __init__(self):
		pass

	def __init__(self, im, bbox = None, depth = 0, blocksize = 32):

		if bbox == None:
			bbox = BBox(0,0, im.size[0], im.size[1])
		self.blocksize = blocksize
		self.depth = depth
		self.bbox = bbox
		self.im = im
		self.children = None
		self.type = BlockType.UNKNOWN

		if bbox.w > self.blocksize and bbox.h > self.blocksize:
			#if we are far enough up the tree to consider splitting:

			#generate the child bboxes
			childboxes = [BBox(bbox.x, bbox.y, bbox.w/2, bbox.h/2),
						  BBox(bbox.x+bbox.w/2, bbox.y, bbox.w - bbox.w/2, bbox.h/2),
						  BBox(bbox.x, bbox.y+bbox.h/2, bbox.w/2, bbox.h-bbox.h/2),
						  BBox(bbox.x+bbox.w/2, bbox.y+bbox.h/2, bbox.w - bbox.w/2, bbox.h-bbox.h/2)]

			#figure out the child image types
			childtypes = [ Analyze(im, childboxes[0]),
						   Analyze(im, childboxes[1]),
						   Analyze(im, childboxes[2]),
						   Analyze(im, childboxes[3])]

			same_type = childtypes[0] == childtypes[1] == childtypes[2] == childtypes[3]
			last_div = bbox.w/2 < self.blocksize or bbox.h /2 < self.blocksize

			if same_type and (childtypes[0] != BlockType.ALPHA or last_div):
				#stop iterating, we've hit the bottom
				self.type = childtypes[0]
			else:
				#otherwise, split up the children
				self.children = (
					QuadTreeNode(im, childboxes[0], depth+1, self.blocksize),
					QuadTreeNode(im, childboxes[1], depth+1, self.blocksize),
					QuadTreeNode(im, childboxes[2], depth+1, self.blocksize),
					QuadTreeNode(im, childboxes[3], depth+1, self.blocksize))

			if self.children and len(self.children) == 4 and self.children[0].type == self.children[1].type == self.children[2].type == self.children[3].type == BlockType.ALPHA:
				self.children = None
				self.type = BlockType.ALPHA
		else:
			self.type = Analyze(im, bbox)

	def __repr__(self):
		if self.children == None:
			return "\t"*self.depth+ BlockName(self.type) + " " +  str(self.bbox)
		else:
			l = [str(x) for x in self.children]
			return "\t"*self.depth + "->\n" + "\n".join(l)

	def printme(self):
		print "\t"*self.depth+ BlockName(self.type) + " " +  str(self.bbox)
		if self.children:
			for child in self.children:
				child.printme()

	def GetBBox(self, fn):
		if self.children != None:
			ret = []
			for child in self.children:
				op = child.GetBBox(fn)
				if op != None:
					ret.extend(op)
			if len(ret) > 0:
				return ret
		elif fn(self):
			return [self.bbox]
		return []


#fix up the list so that adjacent similarly sized images are joined
def doopt(orig):
	while True:
		newlist = optlist(orig)
		if len(newlist) == len(orig):
			return newlist
		orig = newlist

def optlist(orig):
	newlist = []
	for o in orig:
		added = False
		for n in newlist:
			if n.w == o.w and n.x == o.x and (n.y + n.h == o.y or o.y + o.h == n.y):
				newlist.remove(n)
				newlist.append(BBox(n.x, min(o.y, n.y), n.w, o.h+n.h))
				added = True
				break
			if n.h == o.h and n.y == o.y and (n.x + n.w == o.x or o.x + o.w == n.x):
				newlist.remove(n)
				newlist.append(BBox(min(o.x, n.x), n.y, n.w+o.w, o.h))
				added = True
				break

		if not added:
			newlist.append(o)
	return newlist

def GetImageRegions(img, blocksize=16):
	rootNode = QuadTreeNode(img, None, 0, blocksize)
	opaque = doopt(sorted( rootNode.GetBBox(lambda x: x.type == BlockType.OPAQUE), key = lambda x : x.w*x.h,reverse=True))
	alpha = doopt(sorted( rootNode.GetBBox(lambda x: x.type == BlockType.ALPHA), key = lambda x : x.w*x.h,reverse=True))

	return Regions( alpha, opaque )

