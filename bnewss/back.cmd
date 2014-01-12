@echo off
set aname=backbnew.zip
attrib -r %aname%
if exist *.log del *.log
if exist %aname% del %aname%
case -enflrqq *
pr 1 0 zip -rT %aname% * -x \*multimedia* -x \*err -x *fte* -x \*.csm -x \*.exe -x \*.obj -x \*.map -x \*.res -x \*.sym -x \*Cset2pre*
unzip -l %aname%
set aname=
