
FUNCTION(OV_PRINT ALREADY_PRINTED TEXT)

IF("${${ALREADY_PRINTED}}" STREQUAL "")
  MESSAGE(STATUS ${TEXT})
ENDIF("${${ALREADY_PRINTED}}" STREQUAL "")

ENDFUNCTION(OV_PRINT)

