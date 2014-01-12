#ifndef PTC_BNEWSKILL_HPP
#define PTC_BNEWSKILL_HPP

#include "bnews.h"

const USHORT KILL_FROM     = 0;
const USHORT KILL_SUBJECT  = 1;
const USHORT KILL_HEADER   = 2;

typedef class BnewsKill  *pBnewsKill;
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
class BnewsKill
{
private:

    struct KILL {
        OString        group;             // group
        OList<USHORT>  uwhere;            // From:, Subject:, Header:
        lOString       lokill;            // list of regular expressions
    };
    
    typedef KILL *PKILL;

    OList<KILL>  olkill;
    
    pRegex     pregex;
    
public:

    BnewsKill();
    ~BnewsKill();

    ULONG
        init      (),
        isKilled  (PCSZ group, USHORT uwhere, PCSZ string),
        readKill  ();
};

#endif // PTC_BNEWSKILL_HPP