def test1():
    # Test that will always return return
    print "PASS"
    assert True
    
def test2():
# Test that will always return false
    assert False   
def test_getFileSize():
    from nose.tools import ok_, eq_
    filesize=0 
    import mp4,os
    mytestfile="./filesizetest"
    testsize=1024
    with open(mytestfile, "wb") as out:
        out.seek(testsize-1)
        out.write('0')
        out.close()
    with open(mytestfile,"rb") as out:
        filesize=mp4.getFileSize(out)
    eq_(testsize,filesize)
    print "Results ", filesize,testsize
    os.remove(mytestfile)
    
