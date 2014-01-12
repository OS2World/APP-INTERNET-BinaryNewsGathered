#ifndef PTC_BNEWSGROUP_HPP
#define PTC_BNEWSGROUP_HPP

#include "bnews.h"
#include "socket.hpp"

typedef class Group;
typedef class Group  *pGroup;
extern        pGroup  pgroup;

class Group : public Socket
{
private:

    OString
        cmd;

    PCHAR
        buf;

    FILE
        *hfget,                // file handle of articles in *.get.txt
        *hfuue,                // file handle *.uue
        *hflog,                // file handle of fnamelog
        *hfnewstime,           // file handle of newstime
        *hfnewsrc;             // file handle of newsrc

    ULONG
        artno,
        bodylines,
        highwater,
        highwaterxover,
        glo, ghi, gmax,
        ktotal,                // total bytes read
        tart,                  // total articles
        killcnt,               // num of articles killed
        bps,                   // thruput
        bytes;

    OString
        msgid;

    pMyTimer
        pmt;

    pBnewsKill
        pbnewskill;
    
public:

    Group();
    ~Group();

    ULONG
        run          (),
        getNext      (),
        get          (),
        getArt       (),
        getArtbyId   (),
        getArtbyNo   (),
        getBody      (),
        getBodybyId  (),
        getBodybyNo  (),
        getHead      (),
        getHeadbyId  (),
        getHeadbyNo  (),
        getStatbyId  (),
        getStatbyNo  (),
        getXover     (BOOL _usehighwater = FALSE),
        getList      (),
        getHelp      (),
        selectGroup  (),
        quit         (),
        sendCmd      (PCSZ _cmd = NULL),
        getThruput   (),
        setThruput   ();

    VOID
        xrate            (ULONG bytes, MyTimer& mt);

    ULONG  getHighwater  ()        { return highwater; }
    VOID   setHighwater  (ULONG i) { highwater = i; }
};

#endif // PTC_BNEWSGROUP_HPP
