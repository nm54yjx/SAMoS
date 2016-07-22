import numpy as np
from collections import OrderedDict

# This is a vital cellmesh.py method for reading the faces files from samos 
def readfc(fcfile):
    faces= []
    with open(fcfile, 'r') as fc:
        for line in fc:
            face = map(int, line.split())
            faceid, face = face[0], face[1:]
            if len(face) > 3:
                # should be a boundary face
                continue
            faces.append(face)
        simp = np.array(faces)
        boundary = 'not implemented'
    return simp, boundary

def _nanmean(arr):
    return np.mean(arr[~np.isnan(arr)])
            

### These are general methods copied from my command.py module

#print square data to file, first column is int and rest are floats.
def dump(dd, fo, htag='#', outstr=None):
    nc = len(dd.keys())
    fo.write(''.join([htag+' ', '%s\t '*nc, '\n']) % tuple(dd.keys()))
    ddv = dd.values()
    nr = len(ddv[0]) # assumption
    if not outstr:
        outstr = '{}\t '*nc + '\n' 
    for i in range(nr):
        dv = [d[i] for d in ddv]
        fo.write(outstr.format(*dv))

def datdump(dd, fo):
    htag = 'keys:'
    dump(dd, fo, htag=htag)

# This only reads dumps of floats and ints
def readdump(fo, firstc=float):
    headers = fo.next()[1:].split()
    dd = OrderedDict()
    for h in headers:
        dd[h] = []
    for line in fo:
        for i, dat in enumerate(line.split()):
            ev = float
            cdat = ev(dat)
            dd[headers[i]].append( cdat )
    return dd

def dumpargs(dd, fo):
    r1 = max(map(len, dd.keys()))
    argsformat = '{:<%d} {:>10}\n' % r1
    for k, v in dd.items():
        fo.write(argsformat.format(k, v))
    

# debugging

def dirk(A):
    print A
    print dir(A)
    sys.exit()
def shiv(al):
    for a in al:
        print a
        print eval(a)

from matplotlib import pyplot as plt
def plotrange(f, a, b, n=100):
    x = np.linspace(a,b,n+1)
    y = map(f, x)
    plt.plot(x, y)
    plt.show()

#np.set_printoptions(threshold=np.nan)

import contextlib
import cStringIO
@contextlib.contextmanager
def nostdout():
    save_stdout = sys.stdout
    sys.stdout = cStringIO.StringIO()
    yield
    sys.stdout = save_stdout

# want to print a vector object
def dumpvec(vec):
    print omvec(vec)

def scatter(mesh, mesh2):
    arrl = []
    for vh in mesh.vertices():
        arrl.append(omvec(mesh.point(vh)))
    npts = np.column_stack(arrl)
    x = npts[:][0]
    y = npts[:][1]
    plt.scatter(x, y, color='red')

    mesh = mesh2
    arrl = []
    for vh in mesh.vertices():
        arrl.append(omvec(mesh.point(vh)))
    npts = np.column_stack(arrl)
    x = npts[:][0]
    y = npts[:][1]
    plt.scatter(x, y, color='blue')

    plt.show()


# how to print a dictionary containing serious data
def stddict(dd):
    for k, v in dd.items():
        print k
        print v
        print 
    



# Important!
# this is shared code for cellmesh and writemesh for interacting with openmesh

# openmesh has a vector object
# too lazy to use this to convert to numpy arrays
# fixed to three dimensions...

def omvec(vec):
    return np.array([vec[0], vec[1], vec[2]])

def idtopt(mesh, rmuid):
    return omvec(mesh.point(mesh.vertex_handle(rmuid)))


# OrderedSet borrowed from internet
#http://code.activestate.com/recipes/576694/
#see also
#http://stackoverflow.com/questions/1653970/does-python-have-an-ordered-set

import collections

class OrderedSet(collections.MutableSet):

    def __init__(self, iterable=None):
        self.end = end = [] 
        end += [None, end, end]         # sentinel node for doubly linked list
        self.map = {}                   # key --> [key, prev, next]
        if iterable is not None:
            self |= iterable

    def __len__(self):
        return len(self.map)

    def __contains__(self, key):
        return key in self.map

    def add(self, key):
        if key not in self.map:
            end = self.end
            curr = end[1]
            curr[2] = end[1] = self.map[key] = [key, curr, end]

    def discard(self, key):
        if key in self.map:        
            key, prev, next = self.map.pop(key)
            prev[2] = next
            next[1] = prev

    def __iter__(self):
        end = self.end
        curr = end[2]
        while curr is not end:
            yield curr[0]
            curr = curr[2]

    def __reversed__(self):
        end = self.end
        curr = end[1]
        while curr is not end:
            yield curr[0]
            curr = curr[1]

    def pop(self, last=True):
        if not self:
            raise KeyError('set is empty')
        key = self.end[1][0] if last else self.end[2][0]
        self.discard(key)
        return key

    def __repr__(self):
        if not self:
            return '%s()' % (self.__class__.__name__,)
        return '%s(%r)' % (self.__class__.__name__, list(self))

    def __eq__(self, other):
        if isinstance(other, OrderedSet):
            return len(self) == len(other) and list(self) == list(other)
        return set(self) == set(other)
