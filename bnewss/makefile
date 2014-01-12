# nmake v4.00.000 for IBM CSet++ v2.1

#WANT_DEBUG=
#WANT_DYNAMIC_CRT=
#WANT_DYNAMIC_OCL=

!ifdef WANT_DEBUG
CFLAGS=/Fi /G4 /Gm /O- /Q /Wall /W2 /Si /Tdp /Ti /Tm /Tx
LFLAGS=/B"/align:4 /m:full /noi /nol /noo /de"
!else
CFLAGS=/Fi /G4 /Gm /O /Q /Wall /W2 /Si /Tdp
LFLAGS=/B"/align:4 /base:0x10000 /exepack:2 /f /m:full /noi /nol /non /noo /packc /packd"
!endif

!ifdef WANT_DEBUG
LIBS=ocli.lib oipi.lib otcpip.lib setloci.lib
!else
!ifdef WANT_DYNAMIC_CRT
CFLAGS=$(CFLAGS) /Gd
!endif
!ifdef WANT_DYNAMIC_OCL
LIBS=oclii.lib otcpip.lib setloci.lib
!else
LIBS=ocli.lib oipi.lib otcpip.lib setloci.lib
!endif
!endif

CC=icc
REPACK=repack
RC=rc /n /x2
IPFC=ipfc
IPFCPREP=ipfcprep
PREPPARMS=-I..\;Ipf\ -L -W
MSGBIND=msgbind

# include directories
H=.;F:\TOOLKIT\H\LIBC;

# if any of these are out of date the entire project will be built
DEP_H=bnews.h defaults.h bnews.hpp bnewskill.hpp \
      bnewsgroup.hpp bnewsnewsrc.hpp \
      mysort.hpp socket.hpp user.hpp

OBJS=bin\bnews.obj bin\bnewskill.obj \
     bin\bnewsgroup.obj bin\bnewsnewsrc.obj \
     bin\mysort.obj bin\socket.obj bin\user.obj

{.\}.cpp.obj:
  $(CC) $(CFLAGS) /I$(H) /c /Fo$@ $<

{.\}.rc.res:
  $(RC) /r $< $@

default: bin\bnews.exe

ptc: bin\bnews.exe move

bin\bnews.exe : $(OBJS) $(DEP_H) $(@B).def bin\$(@B).res
  $(CC) $(CFLAGS) /I$(H) $(LFLAGS) /Fe$@ $(OBJS) $(LIBS) $(@B).def
  $(MSGBIND) <<
>$@
<$(OCL)\msg\ocl0001.msg
*
<$(OCL)\msg\oip0001.msg
*
<<
!ifndef WANT_DEBUG
  $(REPACK) $@
  del $*.bak
!endif  
  $(RC) $*.res $@

move:  
  @del \ut\bnews.exe&move bin\bnews.exe \ut

bin\bnews.obj      : $(@B).cpp $(@B).hpp $(DEP_H)
bin\bnewskill.obj  : $(@B).cpp $(@B).hpp $(DEP_H)
bin\bnewsgroup.obj : $(@B).cpp $(@B).hpp $(DEP_H)
bin\mysort.obj     : $(@B).cpp $(@B).hpp $(DEP_H)
bin\socket.obj     : $(@B).cpp $(@B).hpp $(DEP_H)
bin\user.obj       : $(@B).cpp $(@B).hpp $(DEP_H)
bin\bnews.res      : $(@B).rc bmp\*.ico
