set(LIBRARY_NAME "Enda")

file(GLOB_RECURSE itertools_src_files ${CMAKE_SOURCE_DIR}/Source/Itertools/*.hpp)
file(GLOB_RECURSE layout_src_files ${CMAKE_SOURCE_DIR}/Source/Layout/*.hpp)
file(GLOB_RECURSE mem_src_files ${CMAKE_SOURCE_DIR}/Source/Mem/*.hpp)
file(GLOB_RECURSE stdutil_src_files ${CMAKE_SOURCE_DIR}/Source/StdUtil/*.hpp)
file(GLOB_RECURSE src_files ${CMAKE_SOURCE_DIR}/Source/*.hpp ${itertools_src_files} ${layout_src_files} ${mem_src_files} ${stdutil_src_files})

source_group("Itertools" FILES ${itertools_src_files})
source_group("Layout" FILES ${layout_src_files})
source_group("Mem" FILES ${mem_src_files})
source_group("StdUtil" FILES ${stdutil_src_files})

add_library(${LIBRARY_NAME} INTERFACE ${src_files})
add_library(Enda::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

# Include module for GNU standard installation directories
include(GNUInstallDirs)

target_include_directories(${LIBRARY_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Source>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Enda>
)

# Install library and executable and export as a set
install(TARGETS
    ${LIBRARY_NAME}
    EXPORT ${LIBRARY_NAME}ExportSet
)

# Install the export set
install(EXPORT ${LIBRARY_NAME}ExportSet
    FILE ${LIBRARY_NAME}Targets.cmake
    NAMESPACE ${LIBRARY_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIBRARY_NAME}
)
