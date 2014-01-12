#ifndef PTC_USER_HPP
#define PTC_USER_HPP

#include "bnews.h"

typedef class User  *pUser;
extern        pUser puser;


class User
{
public:

    OString
        cmd,
        group,
        host,
        authuser,
        authpass;

    OString
        fnameget,
        fnameini,
        fnamekill,
        fnamelog,
        fnamenewsrc,
        fnamenewstime,
        fnameover,
        fnameuue;

    BOOL
        dolog,
        dosort,
        dostdio,
        doverify,
        minbps;

    SHORT
        port;
    
    ULONG
        kwanted,
        verbose;

    pNewsrc
        pn;

public:

    OList<Newsrc>
        olnewsrc;

    User();
    ~User();

    User & init();

    ULONG
        readIni(PCSZ _fnameIni),
        readNewsrc(),
        writeNewsrc();

    VOID
        dumpUser();
};

#endif // PTC_USER_HPP
