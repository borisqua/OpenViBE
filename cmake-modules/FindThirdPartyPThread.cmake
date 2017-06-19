# ---------------------------------
# Finds third party pthread lib
# Adds library to target
# Adds include path
# ---------------------------------

GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_ThirdPartyPThread)

IF(UNIX)

	FIND_LIBRARY(LIB_STANDARD_MODULE_PTHREAD pthread)
	IF(LIB_STANDARD_MODULE_PTHREAD)
		OV_PRINT(OV_PRINTED "  Found pthread...")
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_STANDARD_MODULE_PTHREAD})
		ADD_DEFINITIONS(-DTARGET_HAS_PThread)
	ELSE(LIB_STANDARD_MODULE_PTHREAD)
		OV_PRINT(OV_PRINTED "  FAILED to find pthread...")
	ENDIF(LIB_STANDARD_MODULE_PTHREAD)

ENDIF(UNIX)

SET_PROPERTY(GLOBAL PROPERTY OV_TRIED_ThirdPartyPThread "Yes")
