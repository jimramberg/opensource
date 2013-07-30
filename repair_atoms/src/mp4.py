'''
Created on Jul 26, 2013

@author: ramberg
'''
from atom import parse_atoms, AtomWithChildren,read32
import logging
import os

log = logging.getLogger("mp4")

def getFileSize(mp4file):
    mp4file.seek(0, os.SEEK_END)
    endFile = mp4file.tell()
    mp4file.seek(0, os.SEEK_SET)
    return endFile
class mp4(object):
    '''
    The movie class
    '''
    def readfile(self,mp4filename):   
        mp4file = open(mp4filename, "rb")
        for parent in self.atoms:
            print " Parents", parent.name
            for child in parent.children:
                print "Child", child.name
                if (child.name == "ctts"):
                    print("Composition time offset atom found. Out of order samples possible.")
                if (child.name == "sdtp"):
                    print("Sample dependency flag atom found. I and P frames might need to recover that info.")
                if (child.name == "mvhd"):
                    print("MVHD ")
                    mp4file.seek(child.offset + 12)
                    timescale=read32(mp4file)
                    print "timescale",timescale
                    mp4file.seek(child.offset + 4)
                    duration=read32(mp4file)
                    print "duration",duration       
    def __init__(self,mp4filename):
        ''' Constructor'''
        mp4file = open(mp4filename, "rb")
        FileSize= getFileSize(mp4file)
        print "FileSize",FileSize
        self.atoms = parse_atoms(mp4file,FileSize) 