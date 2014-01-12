#ifndef PTC_BNEWSNEWSRC_HPP
#define PTC_BNEWSNEWSRC_HPP

#include "bnews.h"

typedef class Newsrc  *pNewsrc;
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
class Newsrc
{
public:

    Newsrc() {}
    
    Newsrc(PCSZ group_in, ULONG hixover_in, ULONG hiart_in)
        : group(group_in), hixover(hixover_in), hiart(hiart_in)
    {}

    Newsrc(OString group_in, ULONG hixover_in, ULONG hiart_in)
        : group(group_in), hixover(hixover_in), hiart(hiart_in)
    {}
    
    ~Newsrc() {}

    OString
        group;
    
    ULONG
        hixover,
        hiart;
};

#endif // PTC_BNEWSNEWSRC_HPP
