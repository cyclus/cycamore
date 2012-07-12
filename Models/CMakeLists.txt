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
SET(RNG_INCLUDES "@RNG_INCLUDES@@EXTEND@")
SET(Region_REFS "@Region_REFS@@Region_EXTEND@")
SET(Facility_REFS "@Facility_REFS@@Facility_EXTEND@")
SET(Market_REFS "@Market_REFS@@Market_EXTEND@")
SET(Converter_REFS "@Converter_REFS@@Converter_EXTEND@")
SET(Inst_REFS "@Inst_REFS@@Inst_EXTEND@")
CONFIGURE_FILE(${CYCLUS_CORE_SHARE_DIR}/cyclus.rng.in 
  ${PROJECT_BINARY_DIR}/share/cyclus.rng.in @ONLY)
SET(RNG_INCLUDES ";RNG_INCLUDES;")
STRING(REPLACE ";" "@" RNG_INCLUDES "${RNG_INCLUDES}")
SET(EXTEND "\n\ \ @RNG_INCLUDES@")

SET(Converter_REFS ";Converter_REFS;")
STRING(REPLACE ";" "@" Converter_REFS "${Converter_REFS}")
SET(Converter_EXTEND "\n\ \ \ \ \ \ \ \ @Converter_REFS@")

SET(Inst_REFS ";Inst_REFS;")
STRING(REPLACE ";" "@" Inst_REFS "${Inst_REFS}")
SET(Inst_EXTEND "\n\ \ \ \ \ \ \ \ @Inst_REFS@")

SET(Market_REFS ";Market_REFS;")
STRING(REPLACE ";" "@" Market_REFS "${Market_REFS}")
SET(Market_EXTEND "\n\ \ \ \ \ \ \ \ @Market_REFS@")

SET(Facility_REFS ";Facility_REFS;")
STRING(REPLACE ";" "@" Facility_REFS "${Facility_REFS}")
SET(Facility_EXTEND "\n\ \ \ \ \ \ \ \ @Facility_REFS@")

SET(Region_REFS ";Region_REFS;")
STRING(REPLACE ";" "@" Region_REFS "${Region_REFS}")
SET(Region_EXTEND "\n\ \ \ \ \ \ \ \ @Region_REFS@")

CONFIGURE_FILE(${PROJECT_BINARY_DIR}/share/cyclus.rng.in 
  ${PROJECT_BINARY_DIR}/share/cyclus.rng.in.inst @ONLY)


