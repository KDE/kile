file(GLOB symbols *.png)
set(symbolType "greek")
install(FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType})
