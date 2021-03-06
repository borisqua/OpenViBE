# ---------------------------------
# Finds FFTW3
#
# Sets define if the lib is found, adds include paths, libs
#
# ---------

GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_ThirdPartyFFTW3)

# On windows, we take the itpp one.

IF(WIN32)
	FIND_PATH(PATH_FFTW3 include/fftw3.h PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/itpp)
	IF(PATH_FFTW3)
		SET(FFTW3_FOUND TRUE)
		# Of course the name of the library is this or that, depending on the circumstances ...
		FIND_LIBRARY(FFTW3_TMP NAMES libfftw3-3_win32 PATHS ${PATH_FFTW3}/lib NO_DEFAULT_PATH)
		IF(FFTW3_TMP)
			SET(FFTW3_LIBRARIES	libfftw3-3_win32)
		ELSE(FFTW3_TMP)
			SET(FFTW3_LIBRARIES	libfftw3-3)
		ENDIF(FFTW3_TMP)
		SET(FFTW3_INCLUDE_DIRS ${PATH_FFTW3}/include )
		SET(FFTW3_LIBRARY_DIRS ${PATH_FFTW3}/lib )	
	ENDIF(PATH_FFTW3)
ENDIF(WIN32)

IF(UNIX)
	INCLUDE("FindThirdPartyPkgConfig")
	pkg_check_modules(FFTW3 fftw3)
ENDIF(UNIX)

IF(FFTW3_FOUND)
	SET(FFTW3_LOCATED_LIBS "")
	OV_PRINT(OV_PRINTED "  Found fftw3 includes...")	
	FOREACH(FFTW3_LIB ${FFTW3_LIBRARIES})
		SET(FFTW3_LIB1 "FFTW3_LIB1-NOTFOUND")
		# The list is 'fftw3 m' on Fedora 19. CMake gets confused unless the two following lines are used ... 
		# 1) catch fftw3 if its under dependencies/lib/ (fedora19)
		FIND_LIBRARY(FFTW3_LIB1 NAMES ${FFTW3_LIB} PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib NO_DEFAULT_PATH)
		# 2) catch fftw3 from found paths, and libm from usual system paths, i.e. default path is allowed. (covers other systems + libm on fedora).
		FIND_LIBRARY(FFTW3_LIB1 NAMES ${FFTW3_LIB} PATHS ${FFTW3_LIBRARY_DIRS})
		IF(FFTW3_LIB1)
			OV_PRINT(OV_PRINTED "    [  OK  ] Third party lib ${FFTW3_LIB1}")
			LIST(APPEND FFTW3_LOCATED_LIBS ${FFTW3_LIB1})
		ELSE(FFTW3_LIB1)
			OV_PRINT(OV_PRINTED "    [FAILED] Third party lib ${FFTW3_LIB}")
			SET(FFTW3_FOUND "-NOTFOUND")
			BREAK()
		ENDIF(FFTW3_LIB1)	
	ENDFOREACH(FFTW3_LIB)
ENDIF(FFTW3_FOUND)

IF(FFTW3_FOUND)
	INCLUDE_DIRECTORIES(${FFTW3_INCLUDE_DIRS})
	ADD_DEFINITIONS(${FFTW3_CFLAGS})	
	
	FOREACH(FFTW3_LIB ${FFTW3_LOCATED_LIBS})
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${FFTW3_LIB})
	ENDFOREACH(FFTW3_LIB)

	ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyFFTW3)	
ELSE(FFTW3_FOUND)
	OV_PRINT(OV_PRINTED "  FAILED to find fftw3...")
ENDIF(FFTW3_FOUND)

SET_PROPERTY(GLOBAL PROPERTY OV_TRIED_ThirdPartyFFTW3 "Yes")

