
GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_ThirdPartyNeXus)


IF(WIN32)
	FIND_PATH(PATH_NEXUS NeXusDll.dll PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/sdk-nexus NO_DEFAULT_PATH)
ENDIF(WIN32)

IF(PATH_NEXUS)
	OV_PRINT(OV_PRINTED "  Found MindMedia NeXus dll...")

	INSTALL(PROGRAMS "${PATH_NEXUS}/NeXusDll.dll" "${PATH_NEXUS}/RTInst.dll" DESTINATION "bin")
	ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyNeXus)
ELSE(PATH_NEXUS)
	OV_PRINT(OV_PRINTED "  FAILED to find MindMedia NeXus device dll (optional)")
ENDIF(PATH_NEXUS)

SET_PROPERTY(GLOBAL PROPERTY OV_TRIED_ThirdPartyNeXus "Yes")
