file(GLOB symbols *.png)
SET(symbolType "greek")
INSTALL( FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType} )
