#include "bnewsgroup.hpp"

pGroup pgroup = NULL;
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
Group::Group()
  : Socket(),
    hfget      (NULL),
    hfuue      (NULL),
    hflog      (NULL),
    hfnewstime (NULL),
    hfnewsrc   (NULL),
    artno      (0),
    bodylines  (0),
    highwater  (0),
    highwaterxover(0),
    glo(0), ghi(0), gmax(0),  // lo/hi/max articles in GROUP
    ktotal(0),                // total bytes received (body only)
    tart(0),                  // total articles
    killcnt(0),
    bps(-1),
    bytes(0),
    buf        (new char [SOCKBUFSIZE]),
    pmt        (new MyTimer),
    pbnewskill (new BnewsKill)
{
    ::pgroup = this;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
Group::~Group()
{
    ::pgroup = NULL;
    
    if (hfuue)
        fclose(hfuue);
    
    if (pmt)
        delete pmt;

    if (pbnewskill)
        delete pbnewskill;

    if (buf)
        delete buf;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::run()
{
    try {
        init(); connect(::puser->host, ::puser->port).connectMsg();
        Gets(buf); cout << buf NL;
             if (::puser->cmd == "art" || ::puser->cmd == "get") get();
        else if (::puser->cmd == "help"    ) getHelp();
        else if (::puser->cmd == "list"    ) getList();
        else if (::puser->cmd == "next"    ) getNext();
        else if (::puser->cmd == "overall" ) getXover();
        else if (::puser->cmd == "overnew" ) getXover(TRUE);
        quit();
    }
    catch (OException& e) {
        e.viewError();
        return 1;
    }
    return 0;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getNext()
{
    if (selectGroup() != 0)
        return 1;

    highwater = glo;
    pNewsrc pn = ::puser->olnewsrc.getFirst();
    ULONG fhave = FALSE;
    while (pn != NULL) { //cerr FL "pn->group="<<pn->group NL;
        if (pn->group == ::puser->group) {
            artno = highwater = pn->hiart;
            if (artno < glo)
                artno = highwater = pn->hiart = glo;
            if (artno > ghi)
                artno = highwater = pn->hiart = ghi;
            fhave = TRUE;
            break;
        }
        pn = ::puser->olnewsrc.getNext();
    }
    if (!fhave) {
        pn = new Newsrc(::puser->group, 0, (artno = highwater = glo));
        ::puser->olnewsrc << pn;
    }

    ULONG rc = getStatbyNo();
    if (rc == 423 || rc == 421) {
        return rc;
    }

    PCHAR p = NULL;

    if (::puser->dostdio == FALSE) {
        if ((hfuue = fopen(::puser->fnameuue, "a")) == NULL) {
            cerr FL "" << flush; perror(::puser->fnameuue);
            return 1; }}
    
    if (artno == 1) {                   // highwater is 0 in bnews.newsrc
        if ((rc = sendCmd("HEAD")) != 221)
            return rc;
        if ((p = strchr(buf, ' ')) != NULL)
            highwater = atoi(p);
        if (getHead() == 0) {
            if ((rc = sendCmd("BODY")) != 222)
                return rc;
            if ((rc = getBody()) != 0) {
                pn->hiart = highwater;
                return rc;
            }
        }
        pn->hiart = highwater;
        if ((rc = sendCmd("NEXT")) != 223) {
            return rc;
        }
    }

    if ((rc = sendCmd("NEXT")) != 223)
        return rc;
    
    if ((p = strchr(buf, ' ')) != NULL)
        artno = atoi(p+1);

    while (1) {
        if ((rc = sendCmd("HEAD")) != 221)
            return rc;
        if ((p = strchr(buf, ' ')) != NULL)
            highwater = atoi(p);
        if (getHead() == 0) {
            if ((rc = sendCmd("BODY")) != 222)
                return rc;
            if ((rc = getBody()) != 0) {
                pn->hiart = highwater;
                return rc;
            }
        }
        pn->hiart = highwater;
        if ((rc = sendCmd("NEXT")) != 223) {
            return rc;
        }
    }
    return 0;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::get()
{
    ULONG rc = 0;
    
    if (selectGroup() != 0)
        return 1;
    
    if (::puser->dostdio) {           // read from stdin
        const int LINESIZE = 1024;
        PCHAR line = new char[LINESIZE+1]; *line = 0;
        while(fgets(line, LINESIZE, stdin)) {
            if(!*line) break;
            artno = atoi(line);
            ULONG ul = getArtbyNo(); *line = 0;
        }
    } else {

        OQueue<ULONG> q;
        ULONG i = 0;

        ifstream ifsget(::puser->fnameget, ios::in | ios::nocreate);
        if (!ifsget) {
            cerr FL "" << flush; perror(::puser->fnameget);
            return 1;
        }
        PCHAR line = new char[4096]; i = 0;
        while (ifsget.getline(line, 4095)) {
            PULONG a = new ULONG;
            q.put((*a = atoi(line)));
        }
        delete line; ifsget.close();
        
        if ((hfuue = fopen(::puser->fnameuue, "a")) == NULL) {
            cerr FL "" << flush; perror(::puser->fnameuue);
            return 1;
        }
        if(::puser->doverify) {
            cout << "Verifying articles exist on server" NL;
            ULONG cnt = q.numberOfElements();
            while (cnt) {
                PULONG p = q.get(); artno = *p; q.put(p);
                if ((rc = getStatbyNo()) != 223)
                    rc;
                cnt--;
            }
        }
        while(q.numberOfElements()) {
            PULONG p = q.get();
            highwater = artno = *p;
            getArtbyNo();
            delete p;
        }
    }
    return 0;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getArt()
{
    ULONG  rc = 0;
    ULONG  icurline = 1;

    if (!bodylines) bodylines = 1;

    BOOL gotSubject = FALSE;

    bytes = 0;
    pmt->startTimer();
    while(Gets(buf) == 0) {
        if (::puser->dostdio == TRUE) {
            cout << buf NL;
        } else {
            fprintf(hfuue, "%s\n", buf);
            if (!gotSubject) {
                PCHAR p = NULL;
                if((p = strstr(buf, "Subject: ")) != NULL) {
                    gotSubject = TRUE;
                    cout << "  " << p NL;
                }
            } else {
                ULONG b = strlen(buf);
                ktotal += b;
                xrate(bytes += b, *pmt);
                //if (!rc)
                //    if (ktotal > ::puser->kwanted)
                //        rc = 1;
            }
            icurline++;
        }
    }
    tart++;
    cout NL;
    if (::puser->dostdio == FALSE) {fputc('\n', hfuue);}
    return rc;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getArtbyId()
{
    cmd << "ARTICLE "; cmd + msgid; sendCmd();
    if(*buf != '2') {cout << "  msgid: " << msgid NL; return 1;}
    return getArt();
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getArtbyNo()
{
    cmd << "ARTICLE "; cmd + artno; sendCmd();
    if(*buf != '2') {cout << "  article: " << artno NL; return 1;}
    return getArt();
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getBody()
{
    ULONG  rc = 0, icurline = 1;

    bytes = 0;
    pmt->startTimer();
    while(Gets(buf) == 0) {
        if (::puser->dostdio == TRUE) {
            cout << buf NL;
        } else {
            fprintf(hfuue, "%s\n", buf);
            cout <<"\rLines: "<<icurline<<'/'<<bodylines<<" "<<icurline*100/bodylines<<'%' ;
            ULONG b = strlen(buf);
            ktotal += b;
            xrate(bytes += b, *pmt);
            if (!rc)
                if (ktotal > ::puser->kwanted)
                    rc = 1;
            icurline++;
        }
    }
    tart++;
    cout NL;
    if (::puser->dostdio == FALSE) {fputc('\n', hfuue);}
    return rc;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getBodybyId()
{
    cmd << "BODY "; cmd + msgid; sendCmd();
    if(*buf != '2') {cout << "  msgid: " << msgid NL; return 1;}
    return getBody();
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getBodybyNo()
{
    cmd << "BODY "; cmd + artno; sendCmd();
    if(*buf != '2') {cout << "  article: " << artno NL; return 1;}
    return getBody();
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getHead()
{
    ULONG rc  = 0;
    bodylines = 0;
    lOString lines;
    
    while(Gets(buf) == 0) {
        if (!bodylines) {
            PCHAR p = NULL;
            if((p = strstr(buf, "Lines: ")) != NULL)
                bodylines = atoi(p + 6);
            lines << buf;
        }}
    
    if (pbnewskill) {
        pOString pl = lines.getFirst();
        pl = lines.getFirst();
        while (pl) {
            if (strstr(pl->getText(), "From: ")) {
                if ((rc = pbnewskill->isKilled(::puser->group, KILL_FROM, pl->getText()) != 0))
                    break;
            }
            else if (strstr(pl->getText(), "Subject: ")) {
                if ((rc = pbnewskill->isKilled(::puser->group, KILL_SUBJECT, pl->getText()) != 0))
                    break;
            }
            else {
                if ((rc = pbnewskill->isKilled(::puser->group, KILL_HEADER, pl->getText()) != 0))
                    break;
            }
            pl = lines.getNext();
        }
    }

    if (rc == 0) {
        pOString pl = lines.getFirst();
        while (pl) {
            if (::puser->dostdio == TRUE) {
                cout << pl->getText() NL;
            }
            else {
                fprintf(hfuue, "%s\n", pl->getText());
            }
            pl = lines.getNext();
        }
    } else
        killcnt++;
    
    lines.reset();
    if (::puser->dostdio == FALSE) {fputc('\n', hfuue);}
    if (!bodylines) bodylines = 1;
    return rc;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getHeadbyId()
{
    cmd << "HEAD "; cmd + msgid; sendCmd();
    if(*buf != '2') { cout << "  msgid: " << msgid NL; return 1; }
    return getHead();
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getHeadbyNo()
{
    cmd << "HEAD "; cmd + artno; sendCmd();
    if(*buf != '2') { cout << "  article: " << artno NL; return 1; }
    return getHead();
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getStatbyId()
{
    cmd << "STAT "; cmd + msgid; sendCmd();
    if(*buf != '2') { cout << "  msgid: " << msgid NL; return 1; }
    return 0;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getStatbyNo()
{
    cmd << "STAT "; cmd + artno; sendCmd();
    if(*buf != '2') { cout << "  article: " << artno NL; return 1; }
    return 0;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getXover(BOOL _usehighwater)
{
    if(selectGroup() != 0)
        return 1;

    highwaterxover = glo;
    pNewsrc pn  = ::puser->olnewsrc.getFirst();
    ULONG fhave = FALSE;
    while (pn != NULL) {
        if (pn->group == ::puser->group) {
            //cerr FL "1 pn->group="<<pn->group<<" pn->hixover="<<pn->hixover<<" ghi="<<ghi NL;
            artno = highwaterxover = (pn->hixover + 1);
            if (artno < glo)
                artno = highwaterxover = pn->hixover = glo;
            if (artno > ghi)
                artno = highwaterxover = pn->hixover = ghi;
            fhave = TRUE;
            //cerr FL "2 pn->group="<<pn->group<<" pn->hixover="<<pn->hixover<<" ghi="<<ghi NL;
            break;
        }
        pn = ::puser->olnewsrc.getNext();
    }
    if (!fhave) {
        pn = new Newsrc(::puser->group, (artno = highwaterxover = glo), 0);
        ::puser->olnewsrc << pn;
    }

    if (_usehighwater == TRUE) {
        if (pn->hixover == ghi)
            return 0;
        glo = highwaterxover;
        gmax = (ghi - glo) + 1;
    }
    
    cmd << "XOVER ";

    if(glo < ghi) {
        cmd + glo + "-" + ghi;
    }
    else
        cmd + glo;

    sendCmd(); if(buf[0] != '2') return 1;

    FILE *hf = NULL;
    if(::puser->dostdio == FALSE) {
        PCHAR mode = "w";
        if (_usehighwater == TRUE)
            mode = "a";
        if((hf = fopen(::puser->fnameover, mode)) == NULL) {
            cerr FL "" ; perror(::puser->fnameover);
            return 1; }}
    
    ULONG  ixovercount = 1;

    const ULONG columns = 750;
    ULONG tab = 0;
    bytes = 0;
    pmt->startTimer();
    while(Gets(buf) == 0) {
        buf[columns-1] = 0;
        pn->hixover = highwaterxover = artno = atoi(buf);
        if(::puser->dostdio)
            cout << buf NL;
        else {
            tart++;
            fprintf(hf, "%s\n", buf);
            cout <<" "<<ixovercount<<"/"<<gmax<<" "<<ixovercount*100/gmax<<'%' ;
            ULONG b = strlen(buf);
            ktotal += b;
            xrate(bytes += b, *pmt);
        }
        ixovercount++;
    }
    cout NL;
    if (hf)
        fclose(hf);
    
    //  touch <group>.get.txt create it if it doesn't exist, causes less typing
    //  when cutting and pasting from editor
    if(::puser->dostdio == FALSE) {
        if((hf = fopen(::puser->fnameget, "r")) == NULL) {
            hf = fopen(::puser->fnameget, "w");
            fprintf(hf, "<Place Articles to download here>");
            fclose(hf); }}

    if (::puser->dosort && ::puser->dostdio == FALSE) {
        cout << "Sorting "<<::puser->fnameover NL;
//      MySort *s = new MySort(::puser->fnameover, ixovercount + 1, columns+1);
        MySort *s = new MySort(::puser->fnameover, ixovercount + 1, 8192);
        s->setUseCrlf(TRUE);
        s->setUseTabs(TRUE);
        s->run(); delete s;
    }
    return 0;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getList()
{
    sendCmd("LIST"); if(*buf != '2') return 1;

    if (::puser->dostdio == FALSE) {
        OString fnamelist(::puser->host); fnamelist + ".list.txt";
        ofstream ofs(fnamelist);
        if (!ofs) {
            perror(fnamelist);
            return 1;
        }
        ULONG count = 0;
        bytes = 0;
        pmt->startTimer();
        while (Gets(buf) == 0) {
            ofs << buf << endl;
            cout << ++count ;
            ULONG b = strlen(buf);
            ktotal += b;
            xrate(bytes += b, *pmt);
        }
        cout NL;
        ofs.close();
        MySort *s = new MySort(fnamelist, count+1, 256);
        cout << "Sorting " << fnamelist NL;
        s->setUseCrlf(TRUE);
        s->run(); delete s;
    }
    else { // dostdio
        while (Gets(buf) == 0) {
            cout << buf NL;
        }
    }
    return 0;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getHelp()
{
    cmd = "HELP"; sendCmd(); if(*buf != '1') return 1;

    OString  fnamehelp(::puser->host); fnamehelp + ".help.txt";
    FILE    *hf;
    
    if (::puser->dostdio == FALSE) {
        if((hf = fopen(fnamehelp, "w")) == NULL) {
            cerr FL "" << flush; perror(fnamehelp);
            return 1;}}
    
    while(Gets(buf) == 0) {
        cout << buf NL;
        if (::puser->dostdio == FALSE)
            fprintf(hf, "%s\n", buf);
    }
    if (::puser->dostdio == FALSE)
        fclose(hf);
    return 0;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::selectGroup()
{
    cmd << "GROUP "; cmd + ::puser->group;

    sendCmd(); if(*buf != '2') return 1;

    glo = ghi = gmax = 0;

    PCHAR p = NULL;
    if((p = strchr(buf, ' ')) != NULL) {    // number of msg in group
        gmax = atoi(p);
    }
    if((p = strchr(p+1, ' ')) != NULL) {    // lowest numbered msg
        glo = atoi(p);
        if((p = strchr(p+1, ' ')) != NULL)  // highest numbered msg
            ghi = atoi(p);
    }
  //cerr FL "glo="<<glo<<" ghi="<<ghi<<" gmax="<<gmax NL;
    return 0;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::quit()
{
    return sendCmd("QUIT");
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::sendCmd(PCSZ _cmd)
{
    if (_cmd)
        cmd << _cmd;

    OString ocmd(cmd);
    
    Puts(cmd); cout << cmd NL;
    Gets(buf); cout << buf NL;

    ULONG rc = atoi(buf);

    if (*buf == '2')
        return rc;

    BOOL done = FALSE, hidepass = FALSE;
    
    while (!done) {
        switch (rc) {
        case 450:
            cmd << "AUTHINFO SIMPLE" ;
            break;
        case 350:
            cmd << ::puser->authuser; cmd + " " + ::puser->authpass;
            hidepass = TRUE;
            break;
        case 380:
            cmd << "AUTHINFO GENERIC " ; // ?
            break;
        case 480:
            cmd << "AUTHINFO USER " ; cmd + ::puser->authuser;
            break;
        case 381:
            cmd << "AUTHINFO PASS " ; cmd + ::puser->authpass;
            hidepass = TRUE;
            break;
        case 250:
        case 281:
            cmd << ocmd;
            done = TRUE;
            break;
        default:
            done = TRUE;
            return rc;
        }
        Puts(cmd);
        if (hidepass) {
            cmd.replace(::puser->authpass, "********");
            hidepass = FALSE;
        }
        cout << cmd NL;
        Gets(buf); cout << buf NL;
        rc = atoi(buf);
    }
    return rc;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
VOID  Group::xrate(ULONG bytes, MyTimer& mt)
{
    bps = bytes / mt.elapsedSeconds();
    cout \
        <<" "<<bps/1000<<'.'<<bps%1000/100<<"K/sec"
        <<" "<<ktotal/1000<< "K"
        <<" "<<mt.printElapsedTime()
        <<" "<<::pbnews->pmytimer->printElapsedTime()
        <<" Kept: "<<tart
        <<" Killed: "<<killcnt
        <<"\r"<<flush;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::getThruput()
{
    return bps;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG Group::setThruput()
{
    return (bps == -1 ? bps : bps = (bytes / pmt->elapsedSeconds()));
}
//bnews -cover -giadfw.test -hlocalhost -o | filter |
//bnews -cart  -giadfw.test -hlocalhost -o 2>err >art.uue