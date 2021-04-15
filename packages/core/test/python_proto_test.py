import unittest
 
class TestImport(unittest.TestCase):
 
    def test_protobuf_bad_import(self):
        with self.assertRaises(ImportError): 
            from packages.core.test import this_is_invalid

    def test_protobuf_import(self):
	    from packages.core.test import dummy_pb2
	    self.assert_(True) # No exception raised

if __name__ == '__main__':
    unittest.main()
