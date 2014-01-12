#include "bnewskill.hpp"

pBnewsKill  pbnewskill = NULL;
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
BnewsKill::BnewsKill()
  : pregex(NULL)
{
    ::pbnewskill = this;
    pregex = new Regex;
    init();
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
BnewsKill::~BnewsKill()
{
    ::pbnewskill = NULL;

    if (pregex)
        delete pregex;

    PKILL pk = olkill.getFirst();
    while (pk) {
        pk->uwhere.reset();
        pk->lokill.reset();
        pk = olkill.getNext();
    }
    olkill.reset();
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG BnewsKill::init()
{
    ifstream ifs(puser->fnamekill, ios::in | ios::nocreate);
             if (!ifs) {perror(::puser->fnamekill); return 1;}
    
    PCHAR p = NULL;
    PCHAR line = new CHAR[CCHMAXPATH+1];
    while (ifs.getline(line, CCHMAXPATH)) {    // read the kill file
        OString tmp(strlwr(line));
        tmp.stripLeading(' '); tmp.stripLeading('\x09');
        tmp.stripTrailing(' ');tmp.stripTrailing('\x09');
        if (!*tmp)
            continue;
        if ((p = strchr(tmp, '{')) !=NULL) {
            tmp.rightCut('{');
            tmp.stripTrailing(' ');tmp.stripTrailing('\x09');
            PKILL pk = new KILL;
            pk->group << tmp;
            while (ifs.getline(line, CCHMAXPATH)) {
                tmp << strlwr(line);
                tmp.stripLeading(' ');tmp.stripLeading('\x09');
                tmp.stripTrailing(' ');tmp.stripTrailing('\x09');
                if (!*tmp)
                    continue;
                if (*tmp == '}')
                    break;
                if (strstr(tmp, "from ") != NULL) {
                    PUSHORT pus = new USHORT;
                    *pus = KILL_FROM;
                    pk->uwhere << pus;
                }
                else if (strstr(tmp, "subject ") != NULL) {
                    PUSHORT pus = new USHORT;
                    *pus = KILL_SUBJECT;
                    pk->uwhere << pus;
                }
                else if (strstr(tmp, "header ") != NULL) {
                    PUSHORT pus = new USHORT;
                    *pus = KILL_HEADER;
                    pk->uwhere << pus;
                }
                else
                    continue;
                if ((p = strchr(tmp, ' ')) != NULL)
                    pk->lokill << (p + 1);
            }
            olkill << pk;
        }
    }
    delete line;

    /*{   PKILL pk = olkill.getFirst();
        while (pk) {
            cerr FL "pk->group="<<pk->group NL;
            pOString po = pk->lokill.getFirst();
            PUSHORT  pw = pk->uwhere.getFirst();
            while (po) {
                cerr FL "  pk->uwhere="<<*pw<<" pk->lokill="<<po NL;
                pw = pk->uwhere.getNext();
                po = pk->lokill.getNext();
            }   pk = olkill.getNext(); }} */
    
    return 0;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
ULONG BnewsKill::isKilled(PCSZ group_in, USHORT uwhere_in, PCSZ string_in)
{
    if (!*group_in || !*string_in) return FALSE;

    ULONG rc = FALSE;

    PKILL pk = olkill.getFirst();
    while (pk) {
        pregex->set(group_in, pk->group);
        if (pk->group == "all" || (pregex->compare()) == 0) {
            pregex->setString(string_in);
            pOString pl = pk->lokill.getFirst();
            PUSHORT  pu = pk->uwhere.getFirst();
            while (pl) {
                if (*pu == uwhere_in) {
                    pregex->setPattern(pl->getText());
                    if (pregex->compare() == 0) {
                        cout << "  Killed Expression: "<<pl NL;
                        cout << "  " << string_in NL;
                        if (::puser->verbose) {
                            cerr << "  Killed Expression: "<<pl NL;
                            cerr << "  " << string_in NL;
                        }
                        return 1;
                    }
                }
                pl = pk->lokill.getNext();
                pu = pk->uwhere.getNext();
            }
        }
        pk = olkill.getNext();
    }
    return rc;
}
