CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Here we set a few policies to get rid of warnings on newer cmakes.
# we should update the scripts after upgrading each platform's cmake to 3.x.
if(POLICY CMP0026)
  cmake_policy(SET CMP0026 OLD)
endif()
if(POLICY CMP0043)
  cmake_policy(SET CMP0043 OLD)
endif()
if(POLICY CMP0048)
  cmake_policy(SET CMP0048 OLD)
endif()

PROJECT(OpenViBE)

# These versions are used by the subprojects by default.
# If you wish to maintain specific version numbers for a subproject, please do so in the projects CMakeLists.txt
SET(OV_GLOBAL_VERSION_MAJOR 1)
SET(OV_GLOBAL_VERSION_MINOR 3)
SET(OV_GLOBAL_VERSION_PATCH 0)

# Default is to build to dist/. If you wish a custom install, set your own MAKE_INSTALL_PREFIX when you call CMake. Safest to do under a fakeroot.
#IF(NOT CMAKE_INSTALL_PREFIX)
#ENDIF(NOT CMAKE_INSTALL_PREFIX)

MESSAGE(STATUS "Appending compilation flags...")

SET(OV_CUSTOM_DEPENDENCIES_PATH "${CMAKE_SOURCE_DIR}/dependencies")

ADD_DEFINITIONS("-DOV_PROJECT_NAME=\"${PROJECT_NAME}\"")
ADD_DEFINITIONS("-DOV_VERSION_MAJOR=\"${OV_GLOBAL_VERSION_MAJOR}\"")
ADD_DEFINITIONS("-DOV_VERSION_MINOR=\"${OV_GLOBAL_VERSION_MINOR}\"")
ADD_DEFINITIONS("-DOV_VERSION_PATCH=\"${OV_GLOBAL_VERSION_PATCH}\"")

IF(WIN32)
	# Switch /arch:SSE2 enables vectorization. Remove if your CPU/compiler doesn't support it.
	SET(OV_EIGEN_FLAGS "/arch:SSE2")
	# The following is needed to keep OpenMP from hogging all the cores. Note that this env var works only for VS2013+. 
	# On VS2010, it may be better to disable OpenMP.
	SET(OV_OMP_WAIT_POLICY "PASSIVE")
	# Disable /MP if you don't like VS using all the cores for compilation
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4355 /MP ${OV_EIGEN_FLAGS}")
	ADD_DEFINITIONS("-DNOMINMAX -DBOOST_ALL_NO_LIB")
	# At least Windows Vista target - if you target Windows XP (0x0501), some components may not compile
	ADD_DEFINITIONS("-D_WIN32_WINNT=0x0600")

	# Find out the compiler version, deps may use this
	IF(${CMAKE_MAJOR_VERSION} LESS 3)
		INCLUDE(CMakeDetermineVSServicePack)
		DetermineVSServicePack( MSVC_SERVICE_PACK )
	ELSE(${CMAKE_MAJOR_VERSION} LESS 3)
		SET(MSVC_SERVICE_PACK "unknown")
		# MESSAGE(STATUS "st ${CMAKE_CXX_COMPILER_VERSION}")
		IF(${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "16.0.0")
			SET(MSVC_SERVICE_PACK "vc100")	
		ENDIF()			
		IF(${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "17.0.0")
			SET(MSVC_SERVICE_PACK "vc110")
		ENDIF()		
		IF(${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "18.0.0")
			SET(MSVC_SERVICE_PACK "vc120")
		ENDIF()
	ENDIF(${CMAKE_MAJOR_VERSION} LESS 3)
	STRING(REGEX MATCH "vc120.*" MSVC_VER120 ${MSVC_SERVICE_PACK})
	IF(MSVC_VER120) 
		# Boost 1.55 asio causes several compiler warnings on VS2013 we cannot do much about. try removing the following when boost is upgraded.
		ADD_DEFINITIONS("-D_WINSOCK_DEPRECATED_NO_WARNINGS") 

		# If OpenMP is available, using it will allow Eigen to use multiple cores in matrix math. Only used on VS120,
		# as VS100 allocates the cores too aggressively when OpenMP is enabled.
		INCLUDE("FindOpenMP")
		IF(OPENMP_FOUND)
		  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
		ENDIF(OPENMP_FOUND)
	ENDIF(MSVC_VER120)
	MESSAGE(STATUS "MSVC version is ${MSVC_SERVICE_PACK}")
	
	SET(OV_WIN32_BOOST_VERSION "1_55")

ELSEIF(UNIX)
	# check for c++11 support
	INCLUDE(CheckCXXCompilerFlag)
	CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
	CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
	IF(COMPILER_SUPPORTS_CXX11)
	    # TODO maybe in future the next line will be uncommented
	    # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	ELSEIF(COMPILER_SUPPORTS_CXX0X)
	    # TODO maybe in future the next line will be uncommented
	    # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
	ELSE(COMPILER_SUPPORTS_CXX0X)
		MESSAGE(WARNING "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. In the future this may be required.")
	ENDIF(COMPILER_SUPPORTS_CXX11)

	# If OpenMP is available, using it will allow Eigen to use multiple cores in matrix math.
	INCLUDE("FindOpenMP")
	IF(OPENMP_FOUND)
	  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	ENDIF(OPENMP_FOUND)
	
	# Switch -msse2 enables vectorization. Remove if your CPU/compiler doesn't support it.
	SET(OV_EIGEN_FLAGS "-msse2")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ${OV_EIGEN_FLAGS}")
	# This ENV var is needed to locate our custom IT++ on Linux
	SET(ENV{PKG_CONFIG_PATH} "${OV_CUSTOM_DEPENDENCIES_PATH}/lib/pkgconfig")	

	if (${CMAKE_SYSTEM_PROCESSOR} MATCHES i386|i586|i686)
		set ( BIT_MODE "32")
	else ()
		set ( BIT_MODE "64")
	endif ()

	if(EXISTS "/etc/debian_version")
		set (PLATFORM "Debian")
		ADD_DEFINITIONS("-DTARGET_OS_Linux_Debian")
	endif(EXISTS "/etc/debian_version")

	if(EXISTS "/etc/fedora-release")
		set (PLATFORM "Fedora")
		ADD_DEFINITIONS("-DTARGET_OS_Linux_Fedora")
	endif(EXISTS "/etc/fedora-release")

ELSE(WIN32)
	MESSAGE(WARNING "Warning: unknown platform")
ENDIF(WIN32)

IF(NOT CMAKE_BUILD_TYPE)
	MESSAGE(STATUS "Setting default build type to Release")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF(NOT CMAKE_BUILD_TYPE)

# Print the used compilation parameters (for transparency)
GET_DIRECTORY_PROPERTY(TMP_DEFINITIONS COMPILE_DEFINITIONS)
MESSAGE(STATUS "Compilation flags used at source root: ")
MESSAGE(STATUS "  COMPILE_DEFINITIONS = '${TMP_DEFINITIONS}'")
MESSAGE(STATUS "  CMAKE_CXX_FLAGS = '${CMAKE_CXX_FLAGS}'")
MESSAGE(STATUS "  CMAKE_CXX_FLAGS_RELEASE = '${CMAKE_CXX_FLAGS_RELEASE}'")
MESSAGE(STATUS "  CMAKE_CXX_FLAGS_DEBUG = '${CMAKE_CXX_FLAGS_DEBUG}'")

# if want to compile tests, use the following.
SET(OV_COMPILE_TESTS "true")

# if you don't want an Ogre dependency, use the following. All 3D visualizations and 3D apps will be unavailable. The result will compile faster.
# SET(OV_DISABLE_OGRE "true")

# if you don't want a GTK dependency, set the following. Note that most applications including Acquisition Server and Designer will NOT be built.
# This option is mostly of interest to hackers who are interested in using a minimal OpenViBE kernel in their own projects.
# SET(OV_DISABLE_GTK "true")

# The CoAdapt P300 speller is currently only working in the v0.18 version
# of OpenViBE. It is intended to be replaced by an even more robust and easy 
# to use P300 speller from the same authors.
SET(OV_DISABLE_COADAPT_P300 "1")

# By setting SKIP[_FOLDER]* you can skip a subtree (example: SKIP_A_B_C skips folder a/b/c and all its subfolders if any)

# Skip building documentation
SET(SKIP_DOCUMENTATION "1")

# Python plugin sometimes causes problems; use the following to disable it
#SET(SKIP_CONTRIB_PLUGINS_PROCESSING_PYTHON "1")

# Skipping demos can speed up compilation a lot, if you don't need them
#SET(SKIP_APPLICATIONS_DEMOS "1")
#SET(SKIP_CONTRIB_APPLICATIONS "1")



IF(UNIX)
	# On Linux, the presence of matlab plugin .so can cause crashes in Simple DSP box as the two may use different versions of boost which are
	# then wrongly exposed to the other party due to the dlopen() flag RTLD_GLOBAL. Unfortunately this flag is needed or the python plugin
	# imports no longer work... You may enable the Matlab plugin compilation and possibly set the flag to RTLD_LOCAL in ovCKernelLoader.cpp,
	# but be aware of the consequences.
	MESSAGE(STATUS "Note: Disabling Matlab plugin compilation by default")
	SET(SKIP_PLUGINS_PROCESSING_MATLAB "1")
ENDIF(UNIX)

#SET(SKIP_MODULES_AUTOMATON "1")
#SET(SKIP_MODULES_EBML "1")
#SET(SKIP_MODULES_FS "1")
#SET(SKIP_MODULES_SOCKET "1")
#SET(SKIP_MODULES_STREAM "1")
#SET(SKIP_MODULES_SYSTEM "1")
#SET(SKIP_MODULES_XML "1")
#SET(SKIP_APPLICATIONS_PLATFORM_ACQUISITION-SERVER "1")
#SET(SKIP_APPLICATIONS_PLATFORM_DESIGNER "1")
#SET(SKIP_PLUGINS_PROCESSING_EXAMPLES "1")

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules")

# If you wish you can uncomment the next line and add a file with additional information called OVBranches.cmake to
# the cmake-modules folder. It makes modifying this very CMake file easier.
#INCLUDE("OVCustomSettings")

# a list of all project which will be filled by the directory traversal. This is needed to generate the documentation.
SET_PROPERTY(GLOBAL PROPERTY OV_PROP_CURRENT_PROJECTS "")
SET_PROPERTY(GLOBAL PROPERTY OV_PROP_CURRENT_PROJECTS_BUILD_DIR "")

# Used by the various Find* scripts to locate OpenViBE modules
SET(OV_BASE_DIR ${CMAKE_SOURCE_DIR})

# needed for making visual studio projects when this script is called without CMAKE_BUILD_TYPE
SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG "TARGET_BUILDTYPE_Debug")
SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_RELEASE "TARGET_BUILDTYPE_Release")
SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_RELWITHDEBINFO "TARGET_BUILDTYPE_Release")

# OpenViBE-specific helper functions that are used by the CMakeLists of the subprojects.
#INCLUDE("OvAddSingleProject")
INCLUDE("OvAddProjects")
INCLUDE("OvInstallLaunchScript")
INCLUDE("OvLinkBoostLib")
INCLUDE("OvPrint")

# Setup knowledge of GNU-style install path variables
INCLUDE("GNUInstallDirs")
SET(CMAKE_INSTALL_FULL_DOCDIR "${CMAKE_INSTALL_FULL_BINDIR}/../doc/html")

# Add directories that OpenViBE will use to look for its components runtime, unless overridden by environment variables in the launch scripts.
# These defines should only be used in "openvibe/ov_directories.h".
ADD_DEFINITIONS(-DOV_CMAKE_PATH_ROOT="${CMAKE_INSTALL_PREFIX}")
ADD_DEFINITIONS(-DOV_CMAKE_PATH_BIN="${CMAKE_INSTALL_FULL_BINDIR}")
ADD_DEFINITIONS(-DOV_CMAKE_PATH_DATA="${CMAKE_INSTALL_FULL_DATADIR}/openvibe")
ADD_DEFINITIONS(-DOV_CMAKE_PATH_LIB="${CMAKE_INSTALL_FULL_LIBDIR}")

ADD_SUBDIRECTORY("build-tool/")
# Traverse these directories and build their components
ADD_SUBDIRECTORY("common/")
ADD_SUBDIRECTORY("openvibe/")
ADD_SUBDIRECTORY("kernel/")
ADD_SUBDIRECTORY("modules/")
ADD_SUBDIRECTORY("toolkit/")
ADD_SUBDIRECTORY("plugins/")
ADD_SUBDIRECTORY("applications/")
ADD_SUBDIRECTORY("externals/")
ADD_SUBDIRECTORY("contrib/")
ADD_SUBDIRECTORY("documentation/") # needs to be the last since it uses the list of collected projects

#add dependency to plugin inspector to the documentation
IF(openvibe-documentation)
	ADD_DEPENDENCIES(openvibe-documentation openvibe-plugin-inspector)
ENDIF(openvibe-documentation)

ADD_DEPENDENCIES(openvibe-toolkit openvibe-stimulation-generator )
# make a copy of the dependency script, this is done so we can use the same launch scripts both in win install and win devel situations.
IF(WIN32)
	INSTALL(PROGRAMS ${CMAKE_SOURCE_DIR}/scripts/win32-dependencies.cmd DESTINATION ${CMAKE_INSTALL_FULL_BINDIR} RENAME "openvibe-set-env.cmd")
ENDIF(WIN32)

# ----------------------
# Generate 'external application launcher' script
# ----------------------
IF(WIN32)
	SET(OV_EXT_CMD_FILE "openvibe-external-application-launcher.cmd")
ENDIF(WIN32)
IF(UNIX)
	SET(OV_EXT_CMD_FILE "openvibe-external-application-launcher")
ENDIF(UNIX)

CONFIGURE_FILE("${CMAKE_MODULE_PATH}/launchers/${OV_EXT_CMD_FILE}-base" "${CMAKE_CURRENT_BINARY_DIR}/${OV_EXT_CMD_FILE}")
INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${OV_EXT_CMD_FILE} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

# create the openvibe-ogre-plugins.cfg file for Ogre on UNIX
IF(UNIX)
	execute_process(COMMAND pkg-config --variable=plugindir OGRE OUTPUT_VARIABLE OGRE3D_PLUGIN_PATH)
	CONFIGURE_FILE("${CMAKE_MODULE_PATH}/launchers/openvibe-ogre-plugins.cfg-base" "${CMAKE_CURRENT_BINARY_DIR}/openvibe-ogre-plugins.cfg")
	INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/openvibe-ogre-plugins.cfg DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/openvibe)
ENDIF(UNIX)

