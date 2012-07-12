# ------------------- Add all Concrete Models ----------------------------
FILE(GLOB all_valid_subdirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*/CMakeLists.txt")

FOREACH(dir ${all_valid_subdirs})
  IF(${dir} MATCHES "^([^/]*)//CMakeLists.txt")
    STRING(REGEX REPLACE "^([^/]*)//CMakeLists.txt" "\\1" dir_trimmed ${dir})
    ADD_SUBDIRECTORY(${dir_trimmed})
  ENDIF()
ENDFOREACH(dir)

SET( TestSource ${ModelTestSource} PARENT_SCOPE)

# --------------------- Configure the cyclus.rng file -------------------
CONFIGURE_FILE(${CYCLUS_CORE_SHARE_DIR}/cyclus.rng.in 
  ${PROJECT_BINARY_DIR}/share/cyclus.rng @ONLY)

# --------------------- Configure the cyclus.rng.in file -------------------
SET(RNG_INCLUDES "@RNG_INCLUDES@@EXTEND@")
SET(Region_REFS "@Region_REFS@@Region_EXTEND@")
SET(Facility_REFS "@Facility_REFS@@Facility_EXTEND@")
SET(Market_REFS "@Market_REFS@@Market_EXTEND@")
SET(Converter_REFS "@Converter_REFS@@Converter_EXTEND@")
SET(Inst_REFS "@Inst_REFS@@Inst_EXTEND@")
CONFIGURE_FILE(${CYCLUS_CORE_SHARE_DIR}/cyclus.rng.in 
  ${PROJECT_BINARY_DIR}/share/cyclus.rng.tmp @ONLY)

SET(RNG_INCLUDES ";RNG_INCLUDES;")
STRING(REPLACE ";" "@" RNG_INCLUDES "${RNG_INCLUDES}")
SET(EXTEND "\n\ \ @RNG_INCLUDES@")

update_refs(Facility)
update_refs(Inst)
update_refs(Region)
update_refs(Market)
update_refs(Converter)

CONFIGURE_FILE(${PROJECT_BINARY_DIR}/share/cyclus.rng.tmp 
  ${PROJECT_BINARY_DIR}/share/cyclus.rng.in @ONLY)

