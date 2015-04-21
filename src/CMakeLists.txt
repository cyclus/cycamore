# ------------------- Add all Concrete Agents ----------------------------

USE_CYCLUS("cycamore" "reactor")

USE_CYCLUS("cycamore" "fuel_fab")

USE_CYCLUS("cycamore" "enrichment")

USE_CYCLUS("cycamore" "separations")

USE_CYCLUS("cycamore" "sink")

USE_CYCLUS("cycamore" "source")

USE_CYCLUS("cycamore" "deploy_inst")

USE_CYCLUS("cycamore" "manager_inst")

USE_CYCLUS("cycamore" "growth_region")

USE_CYCLUS("cycamore" "storage")

INSTALL_CYCLUS_MODULE("cycamore" "" "NONE")

SET(TestSource ${cycamore_TEST_CC} PARENT_SCOPE)

# install header files
FILE(GLOB h_files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES ${h_files} DESTINATION include/cycamore COMPONENT cycamore)
