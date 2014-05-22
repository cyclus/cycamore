# ------------------- Add all Concrete Agents ----------------------------

MACRO(ADD_AGENT LIB_ROOT SRC_ROOT)
  
  USE_CYCLUS(${LIB_ROOT} ${SRC_ROOT})

  SET(
    TestSource ${TestSource}
    ${${LIB_ROOT}_TEST_CC}
    ${${LIB_ROOT}_CC}
    )

ENDMACRO()

ADD_AGENT("cycamore" "batch_reactor")

ADD_AGENT("cycamore" "enrichment_facility")

#ADD_AGENT("cycamore" "inpro_reactor")

ADD_AGENT("cycamore" "sink_facility")

ADD_AGENT("cycamore" "source_facility")

ADD_AGENT("cycamore" "deploy_inst")

#ADD_AGENT("cycamore" "manager_inst")

#ADD_AGENT("cycamore" "growth_region")

INSTALL_CYCLUS_MODULE("cycamore" "")

SET( TestSource ${TestSource} PARENT_SCOPE)

# install header files
FILE(GLOB h_files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES ${h_files} DESTINATION include/cycamore COMPONENT cycamore)
