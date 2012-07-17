macro(cyclus_init  _path _name)

  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}${_path})

  # Build the libraries from the CYCAMORE_SRC source files
  ADD_LIBRARY( ${_name}       ${_name}.cpp )
  # Link the libraries to libcycluscore 
  TARGET_LINK_LIBRARIES(${_name} dl ${LIBS})
  SET(CYCAMORE_LIBRARIES ${CYCAMORE_LIBRARIES} ${_name} )
  
  CONFIGURE_FILE(
    ${CYCAMORE_SOURCE_DIR}${_path}/${_name}.rng
    ${PROJECT_BINARY_DIR}/lib${_path}/${_name}.rng
    COPYONLY
    )
  
  install(TARGETS ${_name}
    LIBRARY DESTINATION cyclus/lib${_path}
    COMPONENT ${_path}
    )

  install(FILES "${_name}.rng"
    DESTINATION cyclus/lib${_path}
    COMPONENT "${_path}.rng"
    )
endmacro()
  

macro(cyclus_init_model _type _name)
  SET(MODEL_PATH "/Models/${_type}/${_name}")
  cyclus_init(${MODEL_PATH} ${_name})

  SET(${_type}TestSource ${${_type}TestSource} 
    ${CMAKE_CURRENT_SOURCE_DIR}/${_name}.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/${_name}Tests.cpp 
    PARENT_SCOPE)
endmacro()

macro(find_modules _type)
  SET(TYPE_PATH "${CYCLUS_ROOT_DIR}/lib/Models/${_type}/*/")
  FILE(GLOB all_valid_subdirs ${TYPE_PATH} "*/*.rng")
  FOREACH(dir ${all_valid_subdirs})
    STRING(REPLACE "${CYCLUS_ROOT_DIR}/lib/Models/${_type}/" "" model_name "${dir}" )
    SET(${_type}_REFS ${${_type}_REFS} "<ref name='${model_name}'/>")
    SET(RNG_INCLUDES ${RNG_INCLUDES} "<include href='../lib/Models/${_type}/${model_name}/${model_name}.rng'/>")
  ENDFOREACH(dir) 
endmacro()

macro(find_all_modules)
  find_modules(Facility)
  find_modules(Inst)
  find_modules(Region)
  find_modules(Market)
  find_modules(Converter)
  find_modules(Stub)
  find_modules(StubComm)
endmacro()

macro(clean_includes)
  STRING(REPLACE ";" "\n\ \ " RNG_INCLUDES "${RNG_INCLUDES}")
endmacro()

macro(clean_refs _type)
  STRING(REPLACE ";" "\n\ \ \ \ \ \ \ \ " ${_type}_REFS "${${_type}_REFS}")
endmacro()

macro(clean_all_refs)
  clean_refs(Facility)
  MESSAGE(STATUS "Facility_REFS ${Facility_REFS}") 
  clean_refs(Inst)
  clean_refs(Region)
  clean_refs(Market)
  clean_refs(Converter)
  clean_refs(Stub)
  clean_refs(StubComm)
endmacro()

macro(install_rng)
  find_all_modules()
  clean_includes()
  clean_all_refs()
  CONFIGURE_FILE(${CYCLUS_CORE_SHARE_DIR}/cyclus.rng.in
    ${PROJECT_BINARY_DIR}/share/cyclus.rng @ONLY)
  INSTALL(FILES
    ${PROJECT_BINARY_DIR}/share/cyclus.rng
    DESTINATION ${CYCLUS_CORE_SHARE_DIR}
    COMPONENT data
  )
endmacro()
