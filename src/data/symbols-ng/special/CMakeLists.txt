file(GLOB symbols *.png)
set(symbolType "special")
install(FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType})
