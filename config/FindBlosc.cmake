# - Find BLOSC
# Find the native BLOSC includes and library
# This module defines
#  BLOSC_INCLUDE_DIR, where to find BLOSClib.h, etc.
#  BLOSC_LIBRARIES, the libraries needed to use BLOSC.
#  BLOSC_FOUND, If false, do not try to use BLOSC.
# also defined, but not for general use are
#  BLOSC_LIBRARY, where to find the BLOSC library.

FIND_PATH(BLOSC_INCLUDE_DIR blosc.h)

SET(BLOSC_NAMES ${BLOSC_NAMES} blosc)
FIND_LIBRARY(BLOSC_LIBRARY NAMES ${BLOSC_NAMES})

# handle the QUIETLY and REQUIRED arguments and set BLOSC_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BLOSC DEFAULT_MSG BLOSC_LIBRARY BLOSC_INCLUDE_DIR)

IF(BLOSC_FOUND)
  SET(BLOSC_LIBRARIES ${BLOSC_LIBRARY})
ENDIF(BLOSC_FOUND)

# Deprecated declarations.
SET (NATIVE_BLOSC_INCLUDE_PATH ${BLOSC_INCLUDE_DIR} )
IF(BLOSC_LIBRARY)
  GET_FILENAME_COMPONENT (NATIVE_BLOSC_LIB_PATH ${BLOSC_LIBRARY} PATH)
ENDIF(BLOSC_LIBRARY)
