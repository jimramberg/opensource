'''
Created on Jul 26, 2013

@author: ramberg
'''
from atom import parse_atoms,read32
from ctypes import CDLL, byref, POINTER,c_void_p
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
    def parseTracks(self):
        print (" now in parse tracks")
        for parent in self.atoms:
            if (parent.name == "mdat"):
                print("Mdat atom found")
            if (parent.name == "moov"):
                for child in parent.children:
                    print "Child", child.name
                    if(child.name == "trak"):
                            size = child.size
                            print " Size", size
#                for i in range(0,size):
   #     Track track;
   #     track.codec.context = context->streams[i]->codec;
   #     track.parse(traks[i], mdat);
   #    tracks.push_back(track);


    def readfile(self,mp4filename):   
        MVHD=False
        mp4file = open(mp4filename, "rb")
        avcodec= CDLL("/usr/local/lib/libavcodec.dylib")
        avformat= CDLL("/usr/local/lib/libavformat.dylib")
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
                    MVHD=True
                    mp4file.seek(child.offset + 12)
                    timescale=read32(mp4file)
                    print "timescale",timescale
                    mp4file.seek(child.offset + 4)
                    duration=read32(mp4file)
                    print "duration",duration      
        if (not MVHD):
            raise Exception("Could not find mvhd atom")

        avformat.av_register_all()
        avformat.avformat_alloc_context.restype=POINTER(c_void_p)
        context = avformat.avformat_alloc_context()
        contextref=byref(context)
        print " contextref defined "
        print (" About to open file")
        error = avformat.avformat_open_input(contextref, mp4filename, None, None) 
        print "ERROR is ",error
        if(error != 0):
            raise Exception("Could not parse AV file: " + mp4filename)
        print( " about to run find stream info ")
        if(avformat.avformat_find_stream_info(context, None) < 0):
            raise Exception("Could not find stream info");
        print( " about to run dump format")
   #     avformat.av_dump_format(context, 0,mp4filename, 0);
        self.parseTracks()
        

    def __init__(self,mp4filename):
        ''' Constructor'''
        mp4file = open(mp4filename, "rb")
        FileSize= getFileSize(mp4file)
        print "FileSize",FileSize
        self.atoms = parse_atoms(mp4file,FileSize) 