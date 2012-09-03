from iostream cimport istream, ostream

cdef extern from "../lib/b64/decode.h" namespace "base64":

    cdef cppclass decoder:
        decoder()
        decoder(int buffersize_in)

        int decode(char* code_in, int length_in, char* plaintext_out)
        void init()

        void decode(istream istream_in, ostream ostream_in)