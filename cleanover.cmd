/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 *  Cleanover.cmd
 *  Cleans trash from *.over.txt files created by Bnews.
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-  */

  i = RxFuncAdd('RexxLoadFuncs', 'REXXUTIL', 'RexxLoadFuncs')
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

  fmask = '*.over.txt'                /* File mask of files to clean */

  signal on Halt
  signal on NoValue

  drop file.
  call SysFileTree fmask, 'file', 'f'   /* Find all matching files */
  if file.0 = 0 then do
    say ''
    say 'No matching files:  'fmask
    exit 0
  end

  call SysCurState 'Off'

  call time 'e'

  trashcurrent = ""                       /* debug what gets trashed */

  do fcount = 1 to file.0                 /* Clean all matched files */
    fnameover   = substr(file.fcount, 38)
    fnametmp    = fnameover||'.clean.tmp'
    fnametrash  = fnameover||'.trash'
    notready = stream(fnameover, 'c', 'open')   /* check if file is ready */
    if pos("NOTREADY", notready) \= 0 then do
      say notready' 'fnameover
    end
    else
      call Clean
  end

  say "Elapsed time:  "time('e')
  call SysCurState 'On'
  return 0
  /****************************************************************************/
  /*  End of main                                                             */
  /****************************************************************************/


  /****************************************************************************/
  /*  Clean reads each line of the file looking for lines to keep.            */
  /*  If a line is found it is checked for trash.                             */
  /*  If the line has no trash it is saved.                                   */
  /****************************************************************************/
  Clean:
    cr = '0d'x                      /* carriage return                        */
    call SysFileDelete fnametmp     /* delete old files if they exist         */
    call SysFileDelete fnametrash
    say '' ; say 'Cleaning 'fnameover
    iline     = 0
    ideleted  = 0
    ikept     = 0
    do while lines(fnameover) \= 0
      line0 = linein(fnameover)         /* preserve case in line0             */
      line  = translate(line0)          /* to upper case for comparison       */
      iline = iline + 1                 /* no. lines read                     */

      bodylines = CountTabs()           /* trash articles < MIN_BODY_LINES    */
      if bodylines < MIN_BODY_LINES & bodylines \= -1 then do
        call lineout fnametrash, line0
        ideleted = ideleted + 1         /* no. lines trashed                  */
        call charout , cr'Lines read:  'iline'  Kept:  'ikept'  Trashed:  'ideleted
        iterate
      end

      ikeepline = 0                     /* keep flag                          */
      do i = 1 to keep.0
        if pos(keep.i, line) > 0 then   /* look for things to keep            */
          ikeepline = Filter()          /* determine if its trash             */
      end
      if ikeepline == 1 then do         /* keep good articles                 */
        call lineout fnametmp, line0
        ikept = ikept + 1               /* no. lines kept                     */
      end
      else do                           /* discard trash                      */
        call lineout fnametrash, line0
        ideleted = ideleted + 1         /* no. lines trashed                  */
      end
                                        /* Show progress                      */
      call charout , cr'Lines read:  'iline'  Kept:  'ikept'  Trashed:  'ideleted
    end
    say ''
    call lineout fnameover              /* close files  */
    call lineout fnametrash
    call lineout fnametmp
    fs = stream(fnametrash,'c','query size')
    if fs == 0 then
      call SysFileDelete fnametrash
    call SysFileDelete fnameover
   '@ren 'fnametmp' 'filespec('name', fnameover)
    return 0

  /****************************************************************************/
  /*  Filter looks at each line for trash.                                    */
  /*  Returns: 0 if trash is found.                                           */
  /*           1 if no trash is found.                                        */
  /****************************************************************************/
  Filter:                                 /* Look for trash */
    do j = 1 to trash.0
      if pos(trash.j, line) > 0 then do
        trashcurrent = trash.j
        return 0                          /* found trash */
      end
    end
    return 1                              /* no trash */

  /****************************************************************************/
  /* Count the tabs in the overview to find the number of lines in the        */
  /* article body.                                                            */
  /* Returns: number of lines in the message body                             */
  /*          -1 if error                                                     */
  /****************************************************************************/
  CountTabs: procedure expose line

    i = 1
    bodylines = -1;                 /* error flag, could not find body lines */

    i = pos('09'x||'09'x, line, i)  /* find the two tabs */
    if i == 0 then
      return bodylines

    i = pos('09'x, line, i+2)       /* next tab is the body lines */
    if i == 0 then
      return bodylines

    bodylines = substr(line, i+1, 10) /* extract body lines */

    p = pos('09'x, bodylines)         /* delete remaining */
    if p \= 0 then
      bodylines = delstr(bodylines, p, 99999)

    return bodylines


  Halt:
    say ''
    say 'ouch...'
    exit 1

  NoValue:
    say 'novalue at line 'sigl
    exit 0
