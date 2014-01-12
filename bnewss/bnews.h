#ifndef BNEWS_H
#define BNEWS_H

#define __OCL_RESOLVE_TEMPLATES__

#define INCL_VIO                         // VioScrollUp/VioSetCurPos

#include <ocl.hpp>
#include <oip_client.hpp>
#include <oqueue.hpp>
#include <othread.hpp>
#include <othread.hpp>
#include <ovioapp.hpp>
#include <oxcptvio.hpp>

#pragma define (OList<ULONG>)

const PCHAR PROGNAME = "Binary News Gatherer";
const PCHAR PROGVERS = "v0.96";

#define WinSetTitle WIN16SETTITLE
APIRET16 APIENTRY16 WinSetTitle (HAB hab, PSZ title);

typedef class Bnews;
typedef class BnewsKill;
typedef class Newsrc;
typedef class Group;
typedef class Socket;
typedef class User;

typedef class Bnews      *pBnews;
typedef class BnewsKill  *pBnewsKill;
typedef class Newsrc     *pNewsrc;
typedef class Group      *pGroup;
typedef class Socket     *pSocket;
typedef class User       *pUser;

extern  pBnewsKill        pbnewskill;
extern  pUser             puser;
extern  pNewsrc           pnewsrc;

#include "macros.hpp"
#include "regex.hpp"
#include "mytimer.hpp"
#include "defaults.h"
#include "bnews.hpp"
#include "bnewskill.hpp"
#include "bnewsgroup.hpp"
#include "bnewsnewsrc.hpp"
#include "mysort.hpp"
#include "socket.hpp"
#include "user.hpp"

#endif // BNEWS_H
