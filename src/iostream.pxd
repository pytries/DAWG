from libcpp.string cimport string
from libcpp cimport bool

cdef extern from "<istream>" namespace "std" nogil:
    cdef cppclass istream:
        istream() except +
        istream& read (char* s, int n) except +

    cdef cppclass ostream:
        ostream() except +
        ostream& write (char* s, int n) except +

cdef extern from "<fstream>" namespace "std" nogil:
    cdef cppclass ifstream:
        ifstream() except +
        istream(char* filename) except +
        istream(char* filename, int mode) except +

        bool fail() except +

        void open(char* filename) except +
        void open(char* filename, int mode) except +
        void close() except +

        ifstream& read (char* s, int n) except +


cdef extern from "<sstream>" namespace "std":

    cdef cppclass stringstream:
        stringstream()
        stringstream(string s)
        stringstream(string s, int options)
        string str ()
        stringstream& write (char* s, int n)
        stringstream& seekg (int pos)


cdef extern from "<sstream>" namespace "std::stringstream":

#    int in
    int out
    int binary
