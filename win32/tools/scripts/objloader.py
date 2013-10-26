# taken from the pygame wiki http://pygame.org/wiki/OBJFileLoader

class ObjFile:
    def __init__(self, file):
        """Loads a Wavefront OBJ file. """
        self.vertices = []
        self.normals = []
        self.texcoords = []
        self.faces = []
 
        material = None
        for line in file:
            if line.startswith('#'): continue
            values = line.split()
            if not values: continue
            if values[0] == 'v':
                v = map(float, values[1:4])
                self.vertices.append(v)
            elif values[0] == 'vn':
                v = map(float, values[1:4])
                self.normals.append(v)
            elif values[0] == 'vt':
                self.texcoords.append(map(float, values[1:3]))
            elif values[0] in ('usemtl', 'usemat'):
                material = values[1]
            elif values[0] == 'mtllib':
                pass
                #self.mtl = MTL(values[1])
            elif values[0] == 'f':
                verts= []
                for v in values[1:]:
                    vert = []
                    w = v.split('/')
                    
                    vert.append(int(w[0]))
                    
                    if len(w) >= 2 and len(w[1]) > 0:
                        vert.append(int(w[1]))
                    else:
                        vert.append(0)
                        
                    if len(w) >= 3 and len(w[2]) > 0:
                        vert.append(int(w[2]))
                    else:
                        vert.append(0)
                    verts.append(vert)
                self.faces.append(verts)
