
GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_ThirdPartyMCS)


IF(WIN32)
	FIND_PATH(PATH_MCS nvxmcs.dll PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/sdk-mcs/lib NO_DEFAULT_PATH)
ENDIF(WIN32)

IF(PATH_MCS)
	OV_PRINT(OV_PRINTED "  Found MCS device SDK...")
	INCLUDE_DIRECTORIES(${PATH_MCS}/../include/)
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${PATH_MCS}/nvxmcs.lib)

	INSTALL(PROGRAMS "${PATH_MCS}/nvxmcs.dll" DESTINATION "bin")
	ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyMCS)
ELSE(PATH_MCS)
	OV_PRINT(OV_PRINTED "  FAILED to find MCS device SDK (optional)")
ENDIF(PATH_MCS)

SET_PROPERTY(GLOBAL PROPERTY OV_TRIED_ThirdPartyMCS "Yes")
