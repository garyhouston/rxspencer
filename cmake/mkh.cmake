# Copyright (C) 2007-2012 LuaDist.
# Created by Peter Kapec
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# Please note that the package source code is licensed under its own license.

# simplified reimplementation of included "mkh" script.
function ( MKH INFILENAMES OUTFILENAME INC )
  set ( MATCH " ==[ \t]" )
  if ( INC )
    set ( MATCH " =[ \t]" )
  endif ( INC )
  set ( TEXT "" )
  foreach ( INFILENAME ${INFILENAMES} )
  file ( READ "${INFILENAME}" TEXT_MORE )
  set ( TEXT "${TEXT}${TEXT_MORE}" )
  endforeach ( INFILENAME )
  set ( TEXT "\n${TEXT}" )
  string ( REGEX REPLACE "\n" "\nN" TEXT "${TEXT}" )
  string ( REGEX REPLACE "\nN${MATCH}" "\nY" TEXT "${TEXT}" )
  string ( REGEX REPLACE "\nN[^\n]*" "" TEXT "${TEXT}" )
  string ( REGEX REPLACE "\nY([^\n]*)" "\\1\n" TEXT "${TEXT}" )
  set ( TEXT "#ifdef __cplusplus\nextern \"C\" {\n#endif\n${TEXT}#ifdef __cplusplus\n}\n#endif\n" )
  if ( INC )
    set ( TEXT "#ifndef ${INC}\n#define ${INC}\n${TEXT}#endif\n" )
  endif ( INC )
  file ( WRITE "${OUTFILENAME}" "${TEXT}" )
endfunction ( MKH )
