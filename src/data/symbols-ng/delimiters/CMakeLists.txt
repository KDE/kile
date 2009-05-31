file(GLOB symbols *.png)
SET(symbolType "delimiters")
INSTALL( FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType} )
