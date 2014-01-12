// #define TEST_MYSORT
// icc /G4 /Gd /O /Q /Wall /Tdp /W2 mysort.cpp /B"/align:4 /base:0x10000 /exepack:2 /f /m:full /noi /nol /non /noo /packc /packd"
// gcc -O2 -Wall -Zomf -omysort.exe mysort.cpp -lstdcpp
// bcc -w -O2 mysort.cpp

extern "C" int cmp(const void *a, const void *b);

#include "mysort.hpp"

ULONG sdepth;
BOOL  reverse;
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
MySort::MySort(PCHAR fnameout_in,
               ULONG rows_in, ULONG columns_in,
               ULONG sdepth_in, BOOL reverse_in)
  : fnameout(NULL),
    fnametmp(NULL),
    lines(NULL),
    line(NULL),
    iline(0),
    rows(rows_in),
    columns(columns_in),
    sdepth(sdepth_in),
    reverse(reverse_in),
    usetabs(FALSE)
{
    fnameout = new char[CCHMAXPATH];
    fnametmp = new char[CCHMAXPATH];
    strcpy(fnameout, fnameout_in);
    strcpy(fnametmp, fnameout_in);
    PCHAR p = NULL;
    if((p = strrchr(fnametmp, '.')) != NULL) *p = 0;
    strcat(fnametmp, ".tmp");
    ::sdepth  = sdepth;
    ::reverse = reverse;
    //  cerr FL "MySort rows="<<rows<<" columns="<<columns<<" sdepth="<<sdepth<<" reverse="<<reverse NL;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
MySort::~MySort()
{
    delete fnameout;
    delete fnametmp;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
int MySort::run()
{
    ifstream ifs(fnameout, ios::nocreate);
    if (!ifs) {
        cerr FL "" << flush; perror(fnameout);
        return 1; }
    
    lines = new char *[rows*sizeof(char *)];  // allocate rows
    ULONG i = 0;
    for(i = 0; i < rows; i++)                 // allocate columns
        lines[i] = new char[columns];
    
    line = new char[columns];
    
    while(ifs.getline(line, columns)) {
        if (usetabs) {
            PCHAR p = strchr(line, '\x09') + 1;  //@overview
            ULONG tab = 0;                       //@overview
            if (p)                               //@overview
                tab = p - line;                  //@overview
            if (::sdepth < tab) {                //@overview
                ::sdepth = tab;                  //@overview
#ifdef __DEBUG_ALLOC__
                cerr FL "::sdepth="<<::sdepth NL;//@overview
#endif
            }                                    //@overview
        }        
        if((lines[iline++]=strdup(line)) == NULL) {
            cerr FL "sort: insufficient memory" NL;
            return 1;
        }
    }
    delete line;
    ifs.close();
    
#ifdef __DEBUG_ALLOC__
    cerr FL "::sdepth="<<::sdepth NL;//@overview
#endif
    
    qsort((void *)lines,iline,sizeof(char *),cmp);
    
    ofstream ofs(fnametmp);
    if (!ofs) {
        cerr FL "" NL; perror(fnameout);
        return 1;
    }
    for(i = 0; i < iline; i++) {
        ofs << lines[i];
        if (usecrlf)
            ofs << '\n' ;
    }
    ofs.close();
    
    for(i = 0; i < rows; i++) // delete columns
        delete lines[i];
    delete lines;             // delete rows
    
    if(remove(fnameout) != 0) {
        cerr FL "" NL; perror("remove");
    }
    else if(rename(fnametmp, fnameout) != 0) {
        cerr FL "" NL; perror("rename()");
    }
    return 0;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
extern "C" int cmp(const void *a, const void *b)
{
    int result;
    const char *_a = *(const char **)a;
    const char *_b = *(const char **)b;
    
    /* compare at sdepth if other than zero */
    
    if (sdepth > 0) {
        if (strlen(_a) > sdepth)
            _a += sdepth;
        else
            _a = "";
        if (strlen(_b) > sdepth)
            _b += sdepth;
        else
            _b = "";
    }
    result = stricmp(_a,_b);
    return reverse ? -result : result;
}
// ---------------------------------------------------------------------
//
// ---------------------------------------------------------------------
#ifdef TEST_MYSORT
int main(int argc, char **argv)
{
    if(argc < 3) {
        cout << "\nUsage:  mysort <fnametosort> <rows> <columns=512> <depth=0> <reverse=FALSE>" NL;
        return 1;
    }
    
    char     *fnameout  = argv[1];
    ULONG     rows      = 0,
    columns   = 512,
    depth     = 0,
    reverse   = 0;
    MySort   *mysort    = NULL;
    
    for(int i = 0; i < argc; i++) {
        switch(i) {
        case 1: fnameout  = argv[1];        break;
        case 2: rows      = atoi(argv[2]);  break;
        case 3: columns   = atoi(argv[3]);  break;
        case 4: depth     = atoi(argv[4]);  break;
        case 5: reverse   = atoi(argv[5]);  break;}}
    
    switch(argc) {
    case 3: mysort = new MySort(fnameout, rows);                          break;
    case 4: mysort = new MySort(fnameout, rows, columns);                 break;
    case 5: mysort = new MySort(fnameout, rows, columns, depth);          break;
    case 6: mysort = new MySort(fnameout, rows, columns, depth, reverse); break;
    default:
        cout << "\nUsage:  mysort <fnametosort> <rows> <columns=512> <depth=0> <reverse=0>" NL;
        return 1;}
    
    mysort->run();
    delete mysort;
    
    return 0;
}
#endif  //  TEST_MYSORT
