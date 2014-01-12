#ifndef PTC_SOCKET_HPP
#define PTC_SOCKET_HPP

#define __OCL_RESOLVE_TEMPLATES__
#include <ocl.hpp>
#include <olist.hpp>
#include <ostring.hpp>
#include <oip_client.hpp>
#include "macros.hpp"

typedef class Socket;
typedef class Socket  *pSocket;
extern        pSocket  psocket;

#define SOCKBUFSIZE  4096

class Socket : public OIP_Client
{
private:

    CHAR          buffer1[SOCKBUFSIZE + 1];
    PCHAR         buffer;
    ULONG         bufsize;

public:

    Socket() : OIP_Client(), buffer(NULL), bufsize(0) {}
    ~Socket() {}

    ULONG     Gets(PSZ dest);
    Socket&   Puts(PSZ str);
};

#endif // PTC_SOCKET_HPP
