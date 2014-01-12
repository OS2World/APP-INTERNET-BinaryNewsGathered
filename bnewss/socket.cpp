//#define TEST_SOCKET

// icc /G4 /Gm /Q /Wall /W2 /Tdp /Ti /Tm /Tx socket.cpp oclii.lib otcpip.lib /B"/NOE"

#include "socket.hpp"
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Socket::Gets(PSZ dest)
{
    CHAR         *lfptr;
    INT           stringpos = 0, copysize = 0, flReturnString = 0;
    ULONG         n = SOCKBUFSIZE;
    
    for (;;) {
        // get position of LF in the buffer
        lfptr = (char *) memchr( buffer, '\n', bufsize );
        // if there is a LF in the buffer, then ...
        if ( lfptr ) {
            copysize = lfptr - buffer + 1;
            flReturnString = 1;
        }
        else
            copysize = bufsize;
        // make sure we won't write more than n-1 characters
        if ( copysize > (n - 1) - stringpos ) {
            copysize = (n - 1) - stringpos;
            flReturnString = 1;
        }
        // copy copysize characters from buffer into the string
        memcpy( dest + stringpos, buffer, copysize );
        stringpos += copysize;
        // if we got the whole string, then set buffer to the unused data
        // and return the string
        if ( flReturnString ) {
            if (dest[stringpos-2] == '\r')
                dest[stringpos-2] = '\0';
            else if (dest[stringpos-1] == '\n')
                dest[stringpos-1] = '\0';
            else
                dest[stringpos] = '\0';
            bufsize -= copysize;
            buffer += copysize;
            
            if(*dest == '.' && *(dest+1) == '.') {
                memmove( dest, (dest + 1), copysize-1 );
            }
            else if(*dest == '.' && *(dest+1) == '\0') {
                *dest = '\0';
                return -2;
            }
            return 0;
        }
        // reset buffer and receive more data into buffer
        buffer = buffer1;
        if ((bufsize = client.recv(buffer1, SOCKBUFSIZE)) == -1) {
            return -1;
        }
    }
    return 1;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
Socket& Socket::Puts(PSZ str)
{
    OString tmp(str); tmp + "\r\n";
    client.send(tmp, tmp.length());
    return(*this);
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
#ifdef TEST_SOCKET
int main(int argc, char **argv)
{
    MemoryChecker  memCheck;
    OString        host("127.0.0.1");
    USHORT         port = 119;
    pSocket        sock = new Socket;

    if (argc > 1) {
        host << argv[1];
    }

    cout << OIP::Version().getText() NL;
    
    try {
        sock->init();
        sock->connect(host, port).connectMsg();
        sock->Puts("help");
        OString line(CCHMAXPATH);
        while (sock->Gets(line) == 0) {
            cout << line NL;
        }
        sock->Puts("quit\r\n");
        sock->Gets(line); cout << line NL;
    }
    catch (OException& e) {
        e.viewError();
    }
    
    catch (...) {
        printf(OIP::error(9));
    }

    delete sock;
    return 0;
}
#endif
// F:\Ocl\Include\OIP_Client.hpp
// F:\Ocl\Source\OIP_Client.cpp
// F:\Ocl\Source\OIP_Socket.cpp
// F:\Ocl\Samples\Sockets\Source\Sockc.cpp
