macro(cyclus_init  _path _dir _name)

  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}${_path})

  # Build the cyclus executable from the CYCLUS_SRC source files
  ADD_LIBRARY( ${_dir}       ${_name}.cc )
  # Link the libraries to libcycluscore
  TARGET_LINK_LIBRARIES(${_dir} dl ${LIBS})
  SET(CYCAMORE_LIBRARIES ${CYCAMORE_LIBRARIES} ${_dir} )
  
  install(TARGETS ${_dir}
    LIBRARY DESTINATION lib${_path}
    COMPONENT ${_path}
    )

endmacro()
  
macro(cyclus_init_model _dir _name)
  SET(MODEL_PATH "/cyclus/${_dir}")
  cyclus_init(${MODEL_PATH} ${_dir} ${_name})

  SET(TestSource ${TestSource} 
    ${CMAKE_CURRENT_SOURCE_DIR}/${_name}.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/${_name}_tests.cc
    PARENT_SCOPE)
endmacro()
