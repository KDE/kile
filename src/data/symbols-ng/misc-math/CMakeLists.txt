file(GLOB symbols *.png)
SET(symbolType "misc-math")
INSTALL( FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType} )
