    @echo off

    cd ..
    call mksrc
    cd bnews

    set avers=96
    set aname=bnews%avers%.zip
    set atime=03090106
    if exist %tmpdir%\* del/n %tmpdir%\*

    copy  bnews.ini             %tmpdir%
    copy  bnews.kill            %tmpdir%
    copy  bnews.newsrc          %tmpdir%
    copy  doc\bnews.doc         %tmpdir%
    copy  doc\file_id.diz       %tmpdir%
    copy  \cmd\bnewsfilter.cmd  %tmpdir%
    copy  \cmd\cleanover.cmd    %tmpdir%
    copy  \ut\bnews.exe         %tmpdir%

    %tmpdrive% & cd %tmpdir%

    copy \so\pgm\cpp\tcpip\*.zip

    touchd %atime% *

    call delea.cmd
    case -efnl *
    zip -mT %aname% *

    copy \so\pgm\cpp\tcpip\bnews\doc\upload.template.hobbes bnews%avers%.txt

    touchd %atime% *
    unzip -l %aname%
    set aname=
    set atime=
