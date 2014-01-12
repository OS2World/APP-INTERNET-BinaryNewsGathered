#ifndef PTC_BNEWS_HPP
#define PTC_BNEWS_HPP

#include "bnews.h"

typedef class Bnews  *pBnews;
extern        pBnews  pbnews;

class Bnews : public OVioApp
{
private:

    pGroup    pgroup;
    pUser     puser;

    VOID   thrdFn   ();

public:

    pMyTimer  pmytimer;

    Bnews(const ULONG _argc, PCHAR _argv[]);
    ~Bnews();

    PSZ    isOfType () const {  return("Bnews"); }
    VOID   initArgs (PCSZ argStr);
    VOID   usage    ();
    ULONG  init     ();

    OThread<Bnews>  thrd;
};

#endif
