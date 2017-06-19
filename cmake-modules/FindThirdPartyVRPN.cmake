# ---------------------------------
# Adds VRPN library to the target project
# Adds include path
# Requires FindThirdPartyVRPN_Check.cmake to have been run successfully before
# ---------------------------------

GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_ThirdPartyVRPN)

IF(PATH_VRPN)
	# Find all library paths
	
	IF(WIN32)
		FIND_LIBRARY(LIB_VRPN vrpn PATHS ${PATH_VRPN}/lib NO_DEFAULT_PATH)
		FIND_LIBRARY(LIB_VRPN vrpn PATHS ${PATH_VRPN}/lib)
		IF(LIB_VRPN)
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_VRPN}")		
		ELSE(LIB_VRPN)
			OV_PRINT(OV_PRINTED "    [FAILED] lib vrpn")
			RETURN()
		ENDIF(LIB_VRPN)		
		
		FIND_LIBRARY(LIB_VRPN_DEBUG vrpnd PATHS ${PATH_VRPN}/lib NO_DEFAULT_PATH)
		FIND_LIBRARY(LIB_VRPN_DEBUG vrpnd PATHS ${PATH_VRPN}/lib)			
		IF(LIB_VRPN_DEBUG)
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_VRPN_DEBUG}")			
		ELSE(LIB_VRPN_DEBUG)
			OV_PRINT(OV_PRINTED "    [FAILED] lib vrpnd")
			RETURN()
		ENDIF(LIB_VRPN_DEBUG)	
		
		FIND_LIBRARY(LIB_QUAT quat PATHS ${PATH_VRPN}/lib NO_DEFAULT_PATH)
		FIND_LIBRARY(LIB_QUAT quat PATHS ${PATH_VRPN}/lib)		
		IF(LIB_QUAT)
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_QUAT}")	
		ELSE(LIB_QUAT)
			OV_PRINT(OV_PRINTED "    [FAILED] lib quat")
			RETURN()
		ENDIF(LIB_QUAT)	
		
		FIND_LIBRARY(LIB_QUAT_DEBUG quatd PATHS ${PATH_VRPN}/lib NO_DEFAULT_PATH)
		FIND_LIBRARY(LIB_QUAT_DEBUG quatd PATHS ${PATH_VRPN}/lib)		
		IF(LIB_QUAT_DEBUG)
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_QUAT_DEBUG}")			
		ELSE(LIB_QUAT_DEBUG)
			OV_PRINT(OV_PRINTED "    [FAILED] lib quatd")
			RETURN()
		ENDIF(LIB_QUAT_DEBUG)	
		
	ENDIF(WIN32)

	IF(UNIX)
		FIND_LIBRARY(LIB_VRPNSERVER vrpnserver PATHS ${PATH_VRPN}/lib NO_DEFAULT_PATH)
		FIND_LIBRARY(LIB_VRPNSERVER vrpnserver PATHS ${PATH_VRPN}/lib)
		IF(LIB_VRPNSERVER)
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_VRPNSERVER}")
		ELSE(LIB_VRPNSERVER)
			OV_PRINT(OV_PRINTED "    [FAILED] lib vrpnserver")
			RETURN()
		ENDIF(LIB_VRPNSERVER)

		FIND_LIBRARY(LIB_QUAT quat PATHS ${PATH_VRPN}/lib NO_DEFAULT_PATH)
		FIND_LIBRARY(LIB_QUAT quat PATHS ${PATH_VRPN}/lib)
		IF(LIB_QUAT)
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_QUAT}")
		ELSE(LIB_QUAT)
			OV_PRINT(OV_PRINTED "    [FAILED] lib quat")
			RETURN()
		ENDIF(LIB_QUAT)
	ENDIF(UNIX)
	
	# If we reach here, everything has been found, add
	INCLUDE_DIRECTORIES(${PATH_VRPN}/include)
	
	IF(WIN32)
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} optimized ${LIB_VRPN})	
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} debug ${LIB_VRPN_DEBUG})		
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} optimized ${LIB_QUAT})
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} debug ${LIB_QUAT_DEBUG})		
	ENDIF(WIN32)
	IF(UNIX)
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_VRPNSERVER})	
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_QUAT})		
	ENDIF(UNIX)
	
	ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyVRPN)
ELSE(PATH_VRPN)
	MESSAGE(AUTHOR_WARNING "Should not run FindThirdPartyVRPN without FindThirdPartyVRPN_Check passing successfully first")
ENDIF(PATH_VRPN)

SET_PROPERTY(GLOBAL PROPERTY OV_TRIED_ThirdPartyVRPN "Yes")

