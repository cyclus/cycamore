# ------------------- Add all Concrete Models ----------------------------
FILE(GLOB all_valid_subdirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*/CMakeLists.txt")

# FOREACH(dir ${all_valid_subdirs})
#   IF(${dir} MATCHES "^([^/]*)//CMakeLists.txt")
#     STRING(REGEX REPLACE "^([^/]*)//CMakeLists.txt" "\\1" dir_trimmed ${dir})
#     ADD_SUBDIRECTORY(${dir_trimmed})
#   ENDIF()
# ENDFOREACH(dir)

ADD_SUBDIRECTORY(BatchReactor)
#ADD_SUBDIRECTORY(EnrichmentFacility)
#ADD_SUBDIRECTORY(InproReactor)
#ADD_SUBDIRECTORY(SinkFacility)
ADD_SUBDIRECTORY(SourceFacility)

#ADD_SUBDIRECTORY(DeployInst)
#ADD_SUBDIRECTORY(ManagerInst)
#ADD_SUBDIRECTORY(NullInst)
#
#ADD_SUBDIRECTORY(GrowthRegion)
#ADD_SUBDIRECTORY(NullRegion)

SET( TestSource ${TestSource} PARENT_SCOPE)

