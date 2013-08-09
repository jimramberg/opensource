class Test_getFileSize:
    import logging
    log = logging.getLogger("Test getfilesize")
    def setUp(self):
        print " running Setup",self.testsize
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
        print "Running init"
        self.testsize=testsize
        self.mytestfile = mytestfile
def test_function_gen():
    # need to think of a cleaner way to do this
    sizes=(10240,1024,2000000)
    print "This is the test loop"
    for size in sizes:
        foo=Test_getFileSize("./filetests",size)
        foo.setUp()
        foo.test_getFileSize()
        foo.tearDown()

if __name__ == '__main__' :
    main()

