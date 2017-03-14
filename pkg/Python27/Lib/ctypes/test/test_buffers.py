from ctypes import *
import unittest

class StringBufferTestCase(unittest.TestCase):

    def test_buffer(self):
        b = create_string_buffer(32)
        self.assertEqual(len(b), 32)
        self.assertEqual(sizeof(b), 32 * sizeof(c_char))
        self.assertTrue(type(b[0]) is str)

        b = create_string_buffer("abc")
        self.assertEqual(len(b), 4) # trailing nul char
        self.assertEqual(sizeof(b), 4 * sizeof(c_char))
        self.assertTrue(type(b[0]) is str)
        self.assertEqual(b[0], "a")
        self.assertEqual(b[:], "abc\0")
        self.assertEqual(b[::], "abc\0")
        self.assertEqual(b[::-1], "\0cba")
        self.assertEqual(b[::2], "ac")
        self.assertEqual(b[::5], "a")

    def test_string_conversion(self):
        b = create_string_buffer(u"abc")
        self.assertEqual(len(b), 4) # trailing nul char
        self.assertEqual(sizeof(b), 4 * sizeof(c_char))
        self.assertTrue(type(b[0]) is str)
        self.assertEqual(b[0], "a")
        self.assertEqual(b[:], "abc\0")
        self.assertEqual(b[::], "abc\0")
        self.assertEqual(b[::-1], "\0cba")
        self.assertEqual(b[::2], "ac")
        self.assertEqual(b[::5], "a")

    try:
        c_wchar
    except NameError:
        pass
    else:
        def test_unicode_buffer(self):
            b = create_unicode_buffer(32)
            self.assertEqual(len(b), 32)
            self.assertEqual(sizeof(b), 32 * sizeof(c_wchar))
            self.assertTrue(type(b[0]) is unicode)

            b = create_unicode_buffer(u"abc")
            self.assertEqual(len(b), 4) # trailing nul char
            self.assertEqual(sizeof(b), 4 * sizeof(c_wchar))
            self.assertTrue(type(b[0]) is unicode)
            self.assertEqual(b[0], u"a")
            self.assertEqual(b[:], "abc\0")
            self.assertEqual(b[::], "abc\0")
            self.assertEqual(b[::-1], "\0cba")
            self.assertEqual(b[::2], "ac")
            self.assertEqual(b[::5], "a")

        def test_unicode_conversion(self):
            b = create_unicode_buffer("abc")
            self.assertEqual(len(b), 4) # trailing nul char
            self.assertEqual(sizeof(b), 4 * sizeof(c_wchar))
            self.assertTrue(type(b[0]) is unicode)
            self.assertEqual(b[0], u"a")
            self.assertEqual(b[:], "abc\0")
            self.assertEqual(b[::], "abc\0")
            self.assertEqual(b[::-1], "\0cba")
            self.assertEqual(b[::2], "ac")
            self.assertEqual(b[::5], "a")

if __name__ == "__main__":
    unittest.main()
