'''
Created on Jul 29, 2013

@author: ramberg
Test program
'''
from mp4 import mp4
def main():
    mp4file='/Users/ramberg/Desktop/birthday.MOV'
    foo = mp4(mp4file)
    foo.readfile(mp4file)
if __name__ == '__main__':
    main()