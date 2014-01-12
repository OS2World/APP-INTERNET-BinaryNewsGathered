#include "bnews.hpp"
#include <ctype.h>

// #define USE_THREAD

pBnews pbnews = NULL;
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
class SystemHandler : public OXcptVio
{
public:
    SystemHandler  () : OXcptVio(TRUE) {}
   ~SystemHandler  () {}

    virtual
        PSZ isOfType   () const {return("SystemHandler");}
    
    BOOL SystemHandler::killTrace()
    {
        puser->writeNewsrc();
        if (::pgroup) {
            ::pgroup->Puts("quit");
        }
        if (::pbnews)
            delete ::pbnews;
        cout << "Terminating " << endl;
        unsetHandler();
        cerr << "Errorlevel: 99" NL;
        DosExit(EXIT_THREAD, 99);
        return(FALSE);
    }
    
    BOOL SystemHandler::trapTrace()
    {
        return(FALSE);  // show exception message and abort
    }
};
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
int main(ULONG argc, PCHAR argv[])
{
#if defined (__IBMCPP__) && defined (__DEBUG_ALLOC__)
    MemoryChecker memCheck;
#endif
    OXCPTRECORD   exceptionRecord;
    SystemHandler except;
    except.setHandler(exceptionRecord); // set system exception handler
    
    pBnews pbnews = NULL;
    ULONG rc = 0;
    
    try {
        pbnews = new Bnews(argc, argv);
        if (puser->verbose)
            puser->dumpUser();
        if ((rc = pbnews->init()) != 0) {
            delete pbnews;
            except.unsetHandler();           // release system exception handler
            cerr << "Errorlevel: "<<rc NL;
            return rc;
        }

#ifndef USE_THREAD
        if ((rc = pgroup->run()) == 0)
            puser->writeNewsrc();
#else
        pbnews->thrd.run();

        //pbnews->thrd.waitFor();
        //check thoroughput, if to little then quit
        DosSleep(1000);
        while (pbnews->thrd.isRunning()) {
            DosSleep(5000);
            if (!pbnews->thrd.isRunning()) {
                break;
            }
            if (::pgroup) {
                ULONG t  = pgroup->setThruput();
                cerr FL "t="<<t NL;
                if (t < puser->minbps + 1) {
                    cerr FL "Thoroughput less than "<<puser->minbps+1<<" bps." NL;
                    puser->writeNewsrc();
                    break; }}}
#endif
    }
    catch (OException& e) {
        e.viewError();
    }
    delete pbnews;
    except.unsetHandler();           // release system exception handler
    cerr << "Errorlevel: "<<rc NL;
    return rc;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
Bnews::Bnews(const ULONG _argc, PCHAR _argv[])
  : OVioApp(_argc, _argv),
    pgroup(NULL),
    puser(NULL),
    pmytimer(NULL),
    thrd(this, &Bnews::thrdFn)
{
    ::pbnews = this;
    pmytimer = new MyTimer;
    puser    = new User;
    pgroup   = new Group;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
Bnews::~Bnews()
{
    if (thrd.isRunning()) {
        thrd.stop();
        thrd.waitFor();
    }
    if (pmytimer) {
        cout << "Elapsed: " << pmytimer->printElapsedTime() NL;
        delete pmytimer;
    }
    if (puser->verbose)
        puser->dumpUser();
    delete puser;
    delete pgroup;
    args.reset();
    ::pbnews = NULL;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Bnews::init()
{
    WinSetTitle(0, PROGNAME);

    pmytimer->startTimer();

    pOString p = args.getFirst();
    puser->readIni(puser->fnameini);
    while (p) {
        initArgs(p->getText());
        p = args.getNext();
    }

    if (!puser->cmd) {
        pbnews->usage();
        return 1;
    }

    cout << PROGNAME<<' '<<PROGVERS NL;

    puser->cmd << strlwr(puser->cmd);

    if (puser->cmd != "art"      &&
        puser->cmd != "get"      &&
        puser->cmd != "help"     &&
        puser->cmd != "list"     &&
        puser->cmd != "new"      &&
        puser->cmd != "next"     &&
        puser->cmd != "overall"  &&
        puser->cmd != "overnew"  ) {
        cout << "Unknown command: "<<puser->cmd NL;
        return 1;
    }
    
    if (puser->cmd == "new") {
        cout << "Oops: new not ready.  Use next instead." NL;
        return 1;
    }
    else if ((puser->cmd == "help" || puser->cmd == "list") && !puser->group) {
        puser->group << "dummy.group";
    }
    else if ((puser->cmd != "help" || puser->cmd != "list") && !puser->group) {
        cout << "No group specified: /g<group>" NL;
        return 1;
    }

    puser->fnameget  << puser->group; puser->fnameget  + ".get.txt"  ;
    puser->fnameover << puser->group; puser->fnameover + ".over.txt" ;
    puser->fnameuue  << puser->group; puser->fnameuue  + ".uue"      ;

    // abbreviate filenames
    
    ULONG i = 0;
    const ULONG MAX_FFS = 15;

    struct FFS {
        char *group;
        char *abv;
    };
    typedef FFS *PFFS;

    PFFS *pffs = NULL;

    pffs = new PFFS [MAX_FFS * sizeof(PFFS)];
    for (i = 0; i < MAX_FFS; i++)
        pffs[i] = new FFS;
    
    pffs[ 0]->group = "alt.bbs.doors.binaries."        ; pffs[ 0]->abv = "a.b.d.b." ;
    pffs[ 1]->group = "alt.binaries.erotica."          ; pffs[ 1]->abv = "a.b.e."   ;
    pffs[ 2]->group = "alt.binaries.games."            ; pffs[ 2]->abv = "a.b.g."   ;
    pffs[ 3]->group = "alt.binaries.multimedia."       ; pffs[ 3]->abv = "a.b.m."   ;
    pffs[ 4]->group = "alt.binaries.nude."             ; pffs[ 4]->abv = "a.b.n."   ;
    pffs[ 5]->group = "alt.binaries.pictures.erotica." ; pffs[ 5]->abv = "a.b.p.e." ;
    pffs[ 6]->group = "alt.binaries.pictures."         ; pffs[ 6]->abv = "a.b.p."   ;
    pffs[ 7]->group = "alt.binaries.sounds."           ; pffs[ 7]->abv = "a.b.s."   ;
    pffs[ 8]->group = "alt.binaries.warez."            ; pffs[ 8]->abv = "a.b.w."   ;
    pffs[ 9]->group = "alt.binaries."                  ; pffs[ 9]->abv = "a.b."     ;
    pffs[10]->group = "alt.games."                     ; pffs[10]->abv = "a.g."     ;
    pffs[11]->group = "comp.binaries."                 ; pffs[11]->abv = "c.b."     ;
    pffs[12]->group = "de.alt.binaries."               ; pffs[12]->abv = "d.a.b."   ;
    pffs[13]->group = "fj.binaries."                   ; pffs[13]->abv = "f.b."     ;
    pffs[14]->group = "relcom.comp."                   ; pffs[14]->abv = "r.c."     ;

    puser->fnameover << puser->group;
    
    for (i = 0; i < MAX_FFS; i++) {
        if(strstr(puser->fnameover, pffs[i]->group) != NULL) {
            puser->fnameover.replace(pffs[i]->group, pffs[i]->abv);
            break; }}
    
    for (i = 0; i < MAX_FFS; i++)
        delete pffs[i];
    delete pffs;

    puser->fnameget  << puser->fnameover; puser->fnameget  + ".get.txt"  ;
    puser->fnameuue  << puser->fnameover; puser->fnameuue  + ".uue"      ;
    puser->fnameover <<                   puser->fnameover + ".over.txt" ;

    puser->readNewsrc();
    return 0;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
VOID Bnews::initArgs(PCSZ argStr)
{
    OString arg(argStr);
    PCHAR p = arg;
 // PCHAR p = strlwr(arg);

    *p = tolower(*arg);

    if(*p == '/' || *p == '-')
        *p++;

    *p = tolower(*p);

    if (*p == 'a') {
        if (*p++) {
            *p = tolower(*p);
            if (*p == 'u') {
                if (*(p+1))
                    puser->authuser << (p + 1);
            }
            else if (*p == 'p') {
                if (*(p+1))
                    puser->authpass << (p + 1);
            }
        }
    }
    else if (*p == 'c') {
        if (*(p+1))
            puser->cmd << p+1;
    }
    else if (*p == 'd') {
        puser->verbose = TRUE;
    }
    else if (*p == 'g') {
        if (*(p+1))
            puser->group << p+1;
    }
    else if (*p == 'h') {
        if (*(p+1))
            puser->host << p+1;
    }
    else if (*p == 'k') {
        if (*(p+1))
            puser->fnamekill << p+1;
    }
    else if (*p == 'i') {
        if (*(p+1)) {
            puser->fnameini << p+1; //cerr FL "p+1="<<(PSZ) (p+1) NL; cerr FL "puser->fnameini="<<puser->fnameini NL;
            puser->readIni(puser->fnameini);
        }
    }
    else if (*p == 'l') {
        puser->dolog = TRUE;
        if (*(p+1))
            puser->fnamelog << p+1;
    }
    else if (*p == 'm') {
        if (*(p+1))
            puser->kwanted = (atoi(p+1) * 1000);
    }
    else if (*p == 'o') {
        puser->dostdio = TRUE;
    }
    else if (*p == 'p') {
        if (*(p+1))
            puser->port = (SHORT) atoi(p+1);
    }
    else if (*p == 's') {
        puser->dosort = TRUE;
    }
    else if (*p == 'v') {
        puser->doverify = TRUE;
    }
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
VOID Bnews::thrdFn()
{
    if (pgroup->run() == 0) {
        cerr FL "pgroup->run() = 0 writeNewsrc" NL;
        puser->writeNewsrc();
    }
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
VOID  Bnews::usage()
{
    VioScrollUp(0,0,-1,-1,-1,(PBYTE)"\x20\x07",0);VioSetCurPos(0,0,0);
    cout << \
        PROGNAME<<' '<<PROGVERS<<" Copyright 1996 - 1997 by Crown Software.\n" << \
        CUBUS_COPYRIGHT<<'\n'<<COMPILER<<' '<<COMPILED<<'\n'<<
        "All rights reserved.\n"
        "\n"
        "Usage:  bnews <options>\n"
        "\n"
        "  /A[U|P]<username|password>       - /au<username> | /ap<password>\n"
        "  /C<art|overall|overnew|new|next|list|help>\n"
        "  /G<alt.binaries.pictures>        - the group\n"
        "  /H<news.your.net>                - NNTP news server\n"
        "  /I<my.ini>                       - use my.ini configuration file\n"
        "  /K<killfile>                     - use killfile (default is bnews.kill)\n"
        "  /L<fname.ext>                    - logfile, (default is bnews.log)\n"
        "  /M<maxpacket>                    - max Kbytes to download this session\n"
        "  /O                               - write to stdout/read stdin\n"
        "  /P<xxx>                          - port, default is 119\n"
        "  /S                               - sort the OVERview\n"
        "  /V                               - verify ARTicles exist on server\n"
        "  /D                               - show debug info\n"
        "\n" << flush;
}
