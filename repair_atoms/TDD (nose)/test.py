class Test_getFileSize:
    import logging
    log = logging.getLogger("Test getfilesize")

    def setUp(self):
        print " running Setup"
        with open(self.mytestfile, "wb") as out:
            out.seek(self.testsize-1)
            out.write('0')
            out.close()
    def test_getFileSize(self):
         from nose.tools import ok_, eq_
         import mp4
         with open(self.mytestfile,"rb") as out:
              filesize=mp4.getFileSize(out)
              eq_(self.testsize,filesize,msg='Passed Test  size')
              print "Results ", filesize,self.testsize
    def tearDown(self):
        import os
        print "Running tearDown"
        os.remove(self.mytestfile)
    def __init__(self,mytestfile="./filetest",testsize=102400):
        ''' Constructor'''
        self.testsize=testsize
        self.mytestfile = mytestfile
def main ():
    # need to think of a cleaner way to do this
    # does not work right now.
        Test_getFileSize(1024)
        Test_getFileSize(10240)
if __name__ == '__main__' :
    main()

