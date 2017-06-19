# ---------------------------------
# Finds module System
# Adds library to target
# Adds include path
# ---------------------------------

GET_PROPERTY(OV_PRINTED GLOBAL PROPERTY OV_TRIED_OpenViBEModuleSystem)

OPTION(DYNAMIC_LINK_OPENVIBE_MODULE_SYSTEM "Dynamically link OpenViBE module System" ON)

IF(DYNAMIC_LINK_OPENVIBE_MODULE_SYSTEM)
	SET(OPENVIBE_MODULE_SYSTEM_LINKING "")
ELSE(DYNAMIC_LINK_OPENVIBE_MODULE_SYSTEM)
	SET(OPENVIBE_MODULE_SYSTEM_LINKING "-static")
ENDIF(DYNAMIC_LINK_OPENVIBE_MODULE_SYSTEM)

set(SRC_DIR ${OV_BASE_DIR}/modules/system/include)

FIND_PATH(PATH_OPENVIBE_MODULES_SYSTEM system/defines.h PATHS ${SRC_DIR}  NO_DEFAULT_PATH)
IF(PATH_OPENVIBE_MODULES_SYSTEM)
	OV_PRINT(OV_PRINTED "  Found OpenViBE module System...")
	INCLUDE_DIRECTORIES(${PATH_OPENVIBE_MODULES_SYSTEM}/)


	TARGET_LINK_LIBRARIES(${PROJECT_NAME} openvibe-module-system${OPENVIBE_MODULE_SYSTEM_LINKING})

	ADD_DEFINITIONS(-DTARGET_HAS_System)
ELSE(PATH_OPENVIBE_MODULES_SYSTEM)
	OV_PRINT(OV_PRINTED "  FAILED to find OpenViBE module System...")
ENDIF(PATH_OPENVIBE_MODULES_SYSTEM)

SET_PROPERTY(GLOBAL PROPERTY OV_TRIED_OpenViBEModuleSystem "Yes")

