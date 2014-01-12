#ifndef PTC_MACROS_HPP
#define PTC_MACROS_HPP

#include <iostream.h>
#include <iomanip.h>

#ifndef __EMX__
typedef unsigned long bool;
#endif
typedef unsigned long ulong;

#if defined (__OS2__) && defined (__BORLANDC__)
  #define _System __syscall
  #define _Optlink
  #define COMPILER "Compiler:  Borland C++ v2.0"
#elif defined (__EMX__)
  #define __OS2__
  #define COMPILER "Compiler:  EMX v"<<_emx_vprt
#elif defined (__IBMCPP__)
  #define COMPILER "Compiler:  IBM CSet++ v2.1"
#else
  #define COMPILER "Compiler:  <unknown> macros.hpp line 22"
#endif

#define COMPILED   "Compiled:  "<<__DATE__<<" "<<__TIME__

#define F __FILE__
#define L __LINE__
//#define FL <<dec<<__FILE__<<" "<<setw(14)<<__LINE__<<setw(0)<<" "<<
#define NL <<dec<<'\n'<<flush

#ifndef SETWIDTH
#define SETWIDTH 50
#endif

#define FL << setiosflags(ios::left) << setw(SETWIDTH) << __FILE__ << ' ' << flush << setiosflags(ios::right) << setw(4) << dec << __LINE__ <<' '<<

#define CUBUS_COPYRIGHT \
  "OS/2 Class Library v"<<OCLVER/100<<"."<<OCLVER%100<< \
  " Copyright 1995 - 1997 by Cubus & Team OCL."

#endif  /* PTC_MACROS_HPP */
