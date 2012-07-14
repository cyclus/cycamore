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
  
  SET(RNG_INCLUDES ${RNG_INCLUDES}
    "<include href='../lib${_path}/${_name}.rng'/>"
    PARENT_SCOPE)

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

  SET(${_type}_REFS ${${_type}_REFS}
    "<ref name='${_name}'/>"
    PARENT_SCOPE)
endmacro()


macro(update_refs _type)
  SET(${_type}_REFS ";${_type}_REFS;")
  STRING(REPLACE ";" "@" ${_type}_REFS "${${_type}_REFS}")
  SET(${_type}_EXTEND "@${_type}_REFS@")
endmacro()


macro(update_includes)
  SET(RNG_INCLUDES ";RNG_INCLUDES;")
  STRING(REPLACE ";" "@" RNG_INCLUDES "${RNG_INCLUDES}")
  SET(EXTEND "@RNG_INCLUDES@")
endmacro()

macro(extend_includes)
  SET(RNG_INCLUDES "@RNG_INCLUDES@@EXTEND@")
endmacro()

macro(extend_refs _type)
  SET(${_type}_REFS "@${_type}_REFS@@${_type}_EXTEND@")
endmacro()

macro(dont_extend_refs _type)
  SET(${_type}_REFS "")
  extend_refs(${_type})
endmacro()

macro(extend _type)
  IF(${_type}_REFS MATCHES "ref")
    extend_refs(${_type})
  ELSE(${_type}_REFS MATCHES "ref")
    dont_extend_refs(${_type})
  ENDIF(${_type}_REFS MATCHES "ref")
endmacro()

macro(install_rng_in)
  extend_includes()
  extend(Facility)
  extend(Inst)
  extend(Region)
  extend(Market)
  extend(Converter)
  CONFIGURE_FILE(${CYCLUS_CORE_SHARE_DIR}/cyclus.rng.in
    ${PROJECT_BINARY_DIR}/share/cyclus.rng.tmp @ONLY)
  update_includes()
  update_refs(Facility)
  update_refs(Inst)
  update_refs(Region)
  update_refs(Market)
  update_refs(Converter)
  CONFIGURE_FILE(${PROJECT_BINARY_DIR}/share/cyclus.rng.tmp
    ${PROJECT_BINARY_DIR}/share/cyclus.rng.in @ONLY)
  INSTALL(FILES
    ${PROJECT_BINARY_DIR}/share/cyclus.rng.in
    DESTINATION ${CYCLUS_CORE_SHARE_DIR}
    COMPONENT data
  )
endmacro()


