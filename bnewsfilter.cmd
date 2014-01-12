/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 *  bnewsfilter.cmd
 *
 *  Filter for bnews.
 *
 *  bnews -covernew -o | bnewsfilter | bnews -cart -o > art.uue
 *  bnews -covernew -o | bnewsfilter | bnews -cart -o | uudeview -io -
 *  type <group>.get.txt | bnews -cart -o | uudeview -io -
 *
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

  i = RxFuncAdd('SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs')
  i = SysLoadFuncs()

  MIN_BODY_LINES = 100          /* Trash articles with less than this       */
                                /* number of lines in the message body      */
                                /* to help eliminate discussion and         */
                                /* keep only binary articles.               */

  drop keep.                    /* Stem variable of articles to keep.       */
  drop trash.                   /* "                "        to trash.      */

                                /* keep.0 and trash.0 are the total         */
                                /* number of items in the stem variables    */

  keep.0    = 6                 /* Number of items in stem variable keep.   */
  keep.1    = ".AVI"            /* keep.1 to keep.x are strings to keep     */
  keep.2    = ".MPG"            /* must be UPPER CASE                       */
  keep.3    = ".MOV"
  keep.4    = ".ZIP"
  keep.5    = ".JPG"
  keep.6    = ".GIF"

  trash.0   = 17                /* Number of items in stem variable trash.  */
  trash.1   = "CASH"            /* trash.1 to trash.x are strings to trash  */
  trash.2   = "*** DOT"         /* must be UPPER CASE                       */
  trash.3   = ".WAV"
  trash.4   = "BONDAGE"
  trash.5   = "CLINTON"
  trash.6   = "FREE"
  trash.7   = "GAY"
  trash.8   = "HOT TEEN"
  trash.9   = "NIGGER"
  trash.10  = "SCREENSAVER"
  trash.11  = "SITE"
  trash.12  = "SPAM"
  trash.13  = "STRIP POKER"
  trash.14  = "UPSKIRT"
  trash.15  = "HOT MEN"
  trash.16  = "MCARTH"
  trash.17  = "STUD"

  signal on Halt
  signal on NoValue
 '@echo off'
  parse arg arg0

  drop oline.                       /* overview line after parsing          */
  iread = 0                         /* count of total lines read            */
  isave = 0                         /* count of total lines saved           */
  line0 = ""                        /* line read from stdin                 */
  line  = ""                        /* line used for comparison upper case  */

  call SysCurState 'Off'
  do forever                        /* main loop */
    line0 = linein()                /* preserve case in line0 */
    iread = iread + 1
    if lines() == 0 then leave
    if pos("Elapsed time:", line0) > 0
      then leave
    line = translate(line0)         /* to upper case for comparison */
    oline.0 = ParseLine(line0)

    /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-  */
    /*  After ParseLine, oline contains the following if oline.0 > 1         */
    /*  oline.1   article number                                             */
    /*  oline.2   subject                                                    */
    /*  oline.3   from                                                       */
    /*  oline.4   date/time                                                  */
    /*  oline.5   reply to?                                                  */
    /*  oline.6   msg id?                                                    */
    /*  oline.7   number of bytes in body of message                         */
    /*  oline.8   number of lines in body of message                         */
    /*  oline.9   xref:                                                      */
    /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-  */

    /* this loop only check the subjects for keep/kill strings */

    if oline.0 > 1 then do
      if oline.8 > MIN_BODY_LINES then do
        linecmp = translate(oline.2)
        do i = 1 to keep.0
          if pos(keep.i, linecmp) > 0 then do /* look at subject for things to keep */
            do j = 1 to trash.0               /* look for trash in the subject */
              if pos(trash.j, linecmp) > 0 then do /* found trash, leave */
                i = keep.0
                leave
              end
              else do                         /* no trash, keep line */
                say line0
                isave = isave + 1
                i = keep.0
                leave
              end
            end
          end
        end
      end
    end
    /* couldn't parse tabs, save line just incase */
    else do
      say line0
    end
  end
  return 0
 /* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
 /*                              End of main                            */
 /* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

 /*
  * ParseLine - tokenize line by tabs - 09H
  *
  * results are in the stem variable oline.1 to oline.x
  * oline.0 contains total number of items.
  *
  */

  ParseLine:
    parse arg line
    tabcnt = 1
    tabpos = 1
    tabposlast = 1

    do forever
      tabpos = pos('09'x, line, tabpos)
      if tabpos == 0 then leave
/*    say 'tabcnt='tabcnt' tabpos='tabpos' tabposlast='tabposlast*/
      oline.tabcnt = substr(line, tabposlast + 1, tabpos - tabposlast - 1)
      tabcnt = tabcnt + 1
      tabposlast = tabpos;
      tabpos = tabpos + 1
    end
    oline.tabcnt = substr(line, tabposlast + 1)
    oline.0 = tabcnt
    return oline.0


 /*
  * Ctrl- Break catcher
  */

  Halt:
    call SysCurState 'On'
    say 'ouch... line 'sigl
    exit 1


 /*
  * Initialization error handler
  */

  NoValue:
    say 'no value at line 'sigl
    exit 1
