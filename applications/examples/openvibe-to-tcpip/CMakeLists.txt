PROJECT(openvibe-examples-openvibe-to-tcpip)

SET(PROJECT_VERSION_MAJOR ${OV_GLOBAL_VERSION_MAJOR})
SET(PROJECT_VERSION_MINOR ${OV_GLOBAL_VERSION_MINOR})
SET(PROJECT_VERSION_PATCH ${OV_GLOBAL_VERSION_PATCH})
SET(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

FILE(GLOB_RECURSE source_files src/*.cpp src/*.h src/*.hpp src/*.inl include/*.h)
INCLUDE_DIRECTORIES(include)

ADD_EXECUTABLE(${PROJECT_NAME} ${source_files})

INCLUDE("FindThirdPartyBoost")
INCLUDE("FindThirdPartyBoost_System")
INCLUDE("FindThirdPartyPThread")	# for ubuntu

# ---------------------------------
# Target macros
# Defines target operating system
# Defines target architecture
# Defines target compiler
# ---------------------------------
IF(WIN32)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
	ADD_DEFINITIONS(-DTARGET_OS_Windows)
	ADD_DEFINITIONS(-DTARGET_ARCHITECTURE_i386)
	ADD_DEFINITIONS(-DTARGET_COMPILER_VisualStudio)
	ADD_DEFINITIONS(-D_WIN32_WINNT=0x0501) # for boost::asio
ENDIF(WIN32)
IF(UNIX)
	# ADD_DEFINITIONS(-fvisibility=hidden) # This flag should be present... man gcc
	ADD_DEFINITIONS(-g)
	ADD_DEFINITIONS(-fnon-call-exceptions)
	ADD_DEFINITIONS(-DTARGET_OS_Linux)
	ADD_DEFINITIONS(-DTARGET_ARCHITECTURE_i386)
	ADD_DEFINITIONS(-DTARGET_COMPILER_GCC)
ENDIF(UNIX)

# ----------------------
# Generate launch script
# ----------------------
OV_INSTALL_LAUNCH_SCRIPT("openvibe-examples-openvibe-to-tcpip")

# -----------------------------
# Install files
# -----------------------------
INSTALL(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

INSTALL(DIRECTORY box-tutorials/ DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/openvibe/scenarios/box-tutorials   PATTERN ".svn" EXCLUDE)

#INSTALL(DIRECTORY include/   DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/ FILES_MATCHING PATTERN "*.h" PATTERN ".svn" EXCLUDE)
