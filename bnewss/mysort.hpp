#ifndef PTC_MYSORT_HPP
#define PTC_MYSORT_HPP

#ifdef TEST_MYSORT
#define INCL_NOPMAPI
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream.h>
#include "macros.hpp"
#else
#include "bnews.h"
#endif // TEST_MYSORT

class MySort
{
private:
    
    PCHAR
        fnameout,
        fnametmp;

    CHAR
        **lines,
        *line;
    
    ULONG
        iline;

    ULONG
        rows,         // no. rows to sort
        columns,      // width of row
        sdepth;       // column to start sort at (zero based)
    
    BOOL
        reverse,      // reverse order flag
        usecrlf,      // flag to add crlf to out file
        usetabs;      // flag for overview sort
    
public:
    
    MySort(PCHAR fnameout_in,
           ULONG rows_in,
           ULONG columns_in  = 2048,
           ULONG sdepth_in   = 0,
           BOOL  reverse_in  = FALSE);
    
    ~MySort();
    
    inline VOID   setUseTabs (BOOL set) { usetabs = set; }
    inline VOID   setUseCrlf (BOOL set) { usecrlf = set; }

    int run();
};

#endif  //  PTC_MYSORT_HPP
