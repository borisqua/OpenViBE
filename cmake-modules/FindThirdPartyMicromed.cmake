
GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_ThirdPartyMicromed)


IF(WIN32)
	FIND_PATH(PATH_MICROMED_RELEASE dllMicromed.dll PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/sdk-micromed/Release NO_DEFAULT_PATH)
	FIND_PATH(PATH_MICROMED_DEBUG dllMicromed.dll PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/sdk-micromed/Debug NO_DEFAULT_PATH)	
ENDIF(WIN32)

IF(PATH_MICROMED_RELEASE AND PATH_MICROMED_DEBUG)
	OV_PRINT(OV_PRINTED "  Found Micromed dll...")
						
	INSTALL(PROGRAMS "${PATH_MICROMED_RELEASE}/dllMicromed.dll" DESTINATION "bin" CONFIGURATIONS Release Optimized)
	INSTALL(PROGRAMS "${PATH_MICROMED_DEBUG}/dllMicromed.dll" DESTINATION "bin" CONFIGURATIONS Debug)
	INSTALL(PROGRAMS "${PATH_MICROMED_DEBUG}/dllMicromed.pdb" DESTINATION "bin" CONFIGURATIONS Debug)
	
	ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyMicromed)
ELSE(PATH_MICROMED_RELEASE AND PATH_MICROMED_DEBUG)
	OV_PRINT(OV_PRINTED "  FAILED to find Micromed device dlls (optional)")
ENDIF(PATH_MICROMED_RELEASE AND PATH_MICROMED_DEBUG)

SET_PROPERTY(GLOBAL PROPERTY OV_TRIED_ThirdPartyMicromed "Yes")
