file(GLOB symbols *.png)
SET(symbolType "relation")
INSTALL( FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType} )
