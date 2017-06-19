
GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_ThirdPartyTMSi)

IF(WIN32)
	FIND_PATH(PATH_TMSi TMSiSDK.dll PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/sdk-tmsi NO_DEFAULT_PATH)
ENDIF(WIN32)

IF(PATH_TMSi)
	OV_PRINT(OV_PRINTED "  Found TMSi dll...")

	INSTALL(PROGRAMS "${PATH_TMSi}/TMSiSDK.dll" DESTINATION "bin")
	ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyTMSi)
ELSE(PATH_TMSi)
	OV_PRINT(OV_PRINTED "  FAILED to find TMSi device dll (optional)")
ENDIF(PATH_TMSi)

SET_PROPERTY(GLOBAL PROPERTY OV_TRIED_ThirdPartyTMSi "Yes")
