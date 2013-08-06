'''
Created on Jul 24, 2013

@author: ramberg
'''


def main ():
    import argparse
    global args
    ''' Main option arguments '''
    parser = argparse.ArgumentParser()
    parser.add_argument("-i","--info", help="increase output info",action="store_true",default=False)
    parser.add_argument("-a","--analyze", help="analyze",action="store_true",default=False)
    parser.add_argument("-f","--fix", help="corrupted movie file in need of fixing",action="store",required=True)
    parser.add_argument("-c","--comparison", help="valid movie file to use for input ",action="store",required=True)
    args=parser.parse_args()

try:
    compFile=open(args.comparsion,'r')
    return compFile
except IOError:
        print "Opening ",args.comparison , "failed"
try:
    fixFile=open(args.fix,'r')
    return fixFile
except IOError:
    print "Opening ",args.fix, "failed"       
if(args.analyze):
        print (" Running mp4 printMediaInfo and printAtoms\n")

if(args.info):
    print (" Running mp4 analyze \n")
    
    ''' Now try and do repair and save '''
if __name__ == '__main__':
    