# ---------------------------------
# Finds third party boost
# Adds library to target
# Adds include path
# ---------------------------------

GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_ThirdPartyBoost_FileSystem)

IF(UNIX)
	FIND_LIBRARY(LIB_Boost_Filesystem NAMES "boost_filesystem-mt" PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib NO_DEFAULT_PATH)
	FIND_LIBRARY(LIB_Boost_Filesystem NAMES "boost_filesystem-mt" PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib)

	IF(LIB_Boost_Filesystem)
		OV_PRINT(OV_PRINTED "  Found boost FileSystem...")	
		OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_Boost_Filesystem}")
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_Boost_Filesystem} )
	ELSE(LIB_Boost_Filesystem)
		# Fedora 20 and Ubuntu 13.10,14.04 have no more multi-thread boost libs ( *-mt ) so try if there are non -mt libs to link
		FIND_LIBRARY(LIB_Boost_Filesystem NAMES "boost_filesystem" PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib NO_DEFAULT_PATH)
		FIND_LIBRARY(LIB_Boost_Filesystem NAMES "boost_filesystem" PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib)
		IF(LIB_Boost_Filesystem)
			OV_PRINT(OV_PRINTED "  Found boost FileSystem...")		
			OV_PRINT(OV_PRINTED "    [  OK  ] lib ${LIB_Boost_Filesystem}")
			TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIB_Boost_Filesystem})
		ELSE(LIB_Boost_Filesystem)
			OV_PRINT(OV_PRINTED "  FAILED to find boost FileSystem...")	
			OV_PRINT(OV_PRINTED "    [FAILED] lib boost_filesystem-mt")		
			OV_PRINT(OV_PRINTED "    [FAILED] lib boost_filesystem")
		ENDIF(LIB_Boost_Filesystem)
	ENDIF(LIB_Boost_Filesystem)

	# For Fedora
	INCLUDE("FindThirdPartyPThread")	

ENDIF(UNIX)

IF(WIN32)
	OV_LINK_BOOST_LIB("filesystem" ${OV_WIN32_BOOST_VERSION} )
	OV_LINK_BOOST_LIB("system" ${OV_WIN32_BOOST_VERSION} )			# filesystem depends on system
ENDIF(WIN32)

SET_PROPERTY(GLOBAL PROPERTY OV_TRIED_ThirdPartyBoost_FileSystem "Yes")

