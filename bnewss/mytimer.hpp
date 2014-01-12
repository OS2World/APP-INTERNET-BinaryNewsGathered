#ifndef PTC_MYTIMER_HPP
#define PTC_MYTIMER_HPP

#include <iostream.h>
#include <iomanip.h>
#include <strstrea.h>
#include <time.h>

typedef class MyTimer  *pMyTimer;

class MyTimer
{
private:

    char      achret[80];

    size_t    tstart;

    unsigned long

              days,
              hours,
              minutes,
              seconds,
              total;

public:

    MyTimer() : tstart(0), days(0), hours(0), minutes(0), seconds(0),total(0)
    {}

    ~MyTimer() {}

    inline void startTimer() { tstart = time(NULL); }

    inline size_t  elapsedSeconds()
    {
        size_t s = time(NULL) - tstart;
        return s == 0 ? 1 : s;
    }

    inline char *printElapsedTime()
    {
        total = elapsedSeconds();
        
        // ftp-os2.cdrom.com/.4/os2/unix/uptime.zip
        
        days    = total /  86400 % 365      ;
        hours   = total /  3600 % 24        ;
        minutes = total /  60 % 60          ;
        seconds = total      % 60           ;
        
        ostrstream outstring(achret, sizeof(achret));
        
        // slows Bcos2 -O2 dramatically when compiling
        //if(days)
        //  outstring << days    << " days "
        //            << setw(2) << setfill('0') << hours   << ':'
        //            << setw(2) << setfill('0') << minutes << ':'
        //            << setw(2) << setfill('0') << seconds << flush;
        //else
        outstring \
            << setw(2) << setfill('0') << hours   << ':' \
            << setw(2) << setfill('0') << minutes << ':' \
            << setw(2) << setfill('0') << seconds << '\x0' ;

        return achret;
    }
};

#endif  //  PTC_MYTIMER_HPP
