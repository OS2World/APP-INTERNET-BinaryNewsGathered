#include "user.hpp"

pUser  puser  = NULL;
// ---------------------------------------------------------------------
//                                                                      
// ---------------------------------------------------------------------
User::User()
  : dolog    (DEFAULT_DOLOG),
    dosort   (DEFAULT_DOSORT),
    dostdio  (DEFAULT_DOSTDIO),
    doverify (DEFAULT_DOVERIFY),
    minbps   (DEFAULT_MINBPS),
    port     (DEFAULT_PORT),
    kwanted  ((DEFAULT_KWANTED * 1000)),
    verbose  (DEFAULT_DOVERBOSE),
    host     (getenv("NNTPSERVER")),
    authuser (getenv("USER")),
    authpass (getenv("PASSWORD"))
{
    ::puser = this;

    if (!authuser) 
        authuser << getenv("USERID");

    if (!authpass)
        authpass << getenv("PASS");
    if (!authpass)
        authpass << getenv("PASSIP");

    init();
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
User::~User()
{
    ::puser = NULL;
    olnewsrc.reset();
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
User& User::init()
{
    OString tmp(pbnews->callName); tmp.rightCut('\\');
    fnameini       << tmp  ; fnameini       + "\\bnews.ini"      ;
    fnamekill      << tmp  ; fnamekill      + "\\bnews.kill"     ;
    fnamelog       << tmp  ; fnamelog       + "\\bnews.log"      ;
    fnamenewsrc    << tmp  ; fnamenewsrc    + "\\bnews.newsrc"   ;
    fnamenewstime  << tmp  ; fnamenewstime  + "\\bnews.newstime" ;
    fnameget       << group; fnameget       + ".get.txt"         ;
    fnameover      << group; fnameover      + ".over.txt"        ;
    fnameuue       << group; fnameuue       + ".uue"             ;
    return (*this);
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG User::readIni(PCSZ _fnameini)
{
    fnameini << _fnameini; //cerr FL "fnameini="<<fnameini NL;
    ifstream file(fnameini, ios::in | ios::nocreate);
    if (!file) {cerr FL "" << flush; perror(fnameini); return 1;}
    PCHAR tmp = new char [CCHMAXPATH+1];
    while (file.getline(tmp, CCHMAXPATH)) {
        OString line0(tmp);
        line0.rightCut('\n');
        if(!line0.length())
            continue;
        while (line0[0] == ' ')
            line0.leftCut(' ');
        if (line0[0] == ';')
            continue;

        OString line1(line0); line1 << strlwr(line1); //cerr FL "line0="<<line0<<" line1="<<line1 NL;
        
        PCHAR p = NULL;
        
        if(strstr(line1, "host") != NULL) {
            if((p = strchr(line0, '=')) != NULL)
                host << (char *) (p + 1);
        }
        else if(strstr(line1, "group") != NULL) {
            if((p = strchr(line0, '=')) != NULL)
                group << (char *) (p + 1);
        }
        else if(strstr(line1, "killfile") != NULL) {
            if((p = strchr(line0, '=')) != NULL)
                fnamekill << (char *) (p + 1);
        }
        else if(strstr(line1, "logfile") != NULL) {
            if((p = strchr(line0, '=')) != NULL)
                fnamelog << (char *) (p + 1);
        }
        else if(strstr(line1, "dolog") != NULL) {
            if((p = strchr(line0, '=')) != NULL)
                dolog = atoi(p + 1);
        }
        else if(strstr(line1, "dosort") != NULL) {
            if((p = strchr(line0, '=')) != NULL)
                dosort = atoi(p + 1);
        }
        else if(strstr(line1, "dostdio") != NULL) {
            if((p = strchr(line0, '=')) != NULL)
                dostdio = atoi(p + 1);
        }
        else if(strstr(line1, "doverify") != NULL) {
            if((p = strchr(line0, '=')) != NULL)
                doverify = atoi(p + 1);
        }
        else if(strstr(line1, "debug") != NULL) {
            if((p = strchr(line0, '=')) != NULL)
                verbose = atoi(p + 1);
        }
        else if(strstr(line1, "minbps") != NULL) {
            if((p = strchr(line0, '=')) != NULL)
                minbps = atoi(p + 1);
        }
        else if(strstr(line1, "authuser") != NULL) {
            if((p = strchr(line0, '=')) != NULL)
                authuser << (char *) (p + 1);
        }
        else if(strstr(line1, "authpass") != NULL) {
            if((p = strchr(line0, '=')) != NULL)
                authpass << (char *) (p + 1);
        }
    }
    delete tmp;
    return 0;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG User::readNewsrc()
{
    ifstream ifs(fnamenewsrc, ios::in | ios::nocreate);
    if (!ifs) {cerr FL "" << flush; perror(fnamenewsrc); return 1;}
//  cerr FL "readNewsrc" NL;
    PCHAR tmp = new char[CCHMAXPATH];
    while (ifs.getline(tmp, CCHMAXPATH)) {
        OString line(tmp);
        PCHAR p = strchr(line, ':');
        if (p != NULL) {
            ULONG hixover = 0, hiart = 0;
            if (*(p+1))
                hixover = atoi(p+1);
            if ((p = strchr(p, ',')))
                if (*(p+1))
                    hiart = atoi(p+1);
            line.rightCut(':');
            BOOL fhave = FALSE;
            pn =  olnewsrc.getFirst();
            while (pn != NULL) {
                if (pn->group == line) {
                    fhave = TRUE;
                    pn->hixover = hixover;
                    pn->hiart = hiart;
                    //cerr FL "fhave "<<line<<' '<<pn->hixover<<' '<<pn->hiart NL;
                    break;
                }   pn = olnewsrc.getNext();
            }
            if (fhave == FALSE) {
                pNewsrc pn = new Newsrc(line, hixover, hiart);
                olnewsrc << pn;
                //cerr FL "new: pn->group="<<pn->group<<" pn->hixover="<<pn->hixover<<" pn->hiart="<<pn->hiart NL;
            }
        }
    }
    delete tmp;
    return 0;
}
// ---------------------------------------------------------------------
// If two or more copies of bnews are running, care must be taken
// when updating the newsrc file so that each copy will write the
// highwater marks properly.
// 1.  Save a copy of the highwater mark
// 2.  re-Read newsrc
// 3.  Replace the old highwater mark with the saved highwater mark
// 4.  Write newsrc
//
// Todo:  Access to newsrc should be serialized with mutex semaphores
// ---------------------------------------------------------------------
ULONG User::writeNewsrc()
{
//  cerr FL "writeNewsrc" NL;
    ULONG    tmphi = 0;

    pn = olnewsrc.getFirst();
    while (pn != NULL) {
        if (pn->group == group) {
            if (cmd == "overall" || cmd == "overnew")
                tmphi = pn->hixover;
            else if (cmd == "next")
                tmphi = pn->hiart;
//          cerr FL "tmp: pn->group="<<pn->group<<" tmphi="<<tmphi<<' '<<pn->hixover<<' '<<pn->hiart<<" cmd="<<cmd NL;
            break;
        }
        pn = olnewsrc.getNext();
    }
    if (readNewsrc() != 0) {cerr FL "" NL; return 1;}
    
    ofstream ofs(fnamenewsrc);
    if (!ofs) {cerr FL "" << flush; perror(fnamenewsrc); return 1;}
    pn = olnewsrc.getFirst();
    while (pn != NULL) {
        if (pn->group == group) {
//          cerr FL "cmd="<<cmd NL;
            if (cmd == "overall" || cmd == "overnew")
                pn->hixover = tmphi;
            else if (cmd == "next")
                pn->hiart = tmphi;
        }
//      cerr FL "write: pn->group="<<pn->group<<" pn->hixover="<<pn->hixover<<" pn->hiart="<<pn->hiart NL;
        ofs << pn->group << ": " << pn->hixover << ", " <<pn->hiart NL;
        pn = olnewsrc.getNext();
    }
    return 0;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
VOID User::dumpUser()
{
    cerr << "様 User settings 様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様" NL;
    cerr << "puser->cmd            = "<<puser->cmd            NL;
    cerr << "puser->host           = "<<puser->host           NL;
    cerr << "puser->fnameini       = "<<puser->fnameini       NL;
    cerr << "puser->fnamekill      = "<<puser->fnamekill      NL;
    cerr << "puser->fnamelog       = "<<puser->fnamelog       NL;
    cerr << "puser->fnamenewsrc    = "<<puser->fnamenewsrc    NL;
    cerr << "puser->fnamenewstime  = "<<puser->fnamenewstime  NL;
    cerr << "puser->group          = "<<puser->group          NL;
    cerr << "puser->fnameget       = "<<puser->fnameget       NL;
    cerr << "puser->fnameover      = "<<puser->fnameover      NL;
    cerr << "puser->fnameuue       = "<<puser->fnameuue       NL;
    cerr << "puser->kwanted        = "<<puser->kwanted        NL;
    cerr << "puser->minbps         = "<<puser->minbps         NL;
    cerr << "puser->dolog          = "<<puser->dolog          NL;
    cerr << "puser->dosort         = "<<puser->dosort         NL;
    cerr << "puser->dostdio        = "<<puser->dostdio        NL;
    cerr << "puser->doverify       = "<<puser->doverify       NL;
    cerr << "puser->port           = "<<puser->port           NL;
    cerr << "puser->verbose        = "<<puser->verbose        NL;
    cerr << "puser->authuser       = "<<puser->authuser       NL;
    cerr << "puser->authpass       = "<<puser->authpass       NL;
    pNewsrc pn = olnewsrc.getFirst();
    while (pn) {
    cerr << "pn->group             = "<<pn->group<<" pn->hixover="<<pn->hixover<<" pn->hiart="<<pn->hiart NL;
    pn = olnewsrc.getNext();
    }
}
