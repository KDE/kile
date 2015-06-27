file(GLOB symbols *.png)
set(symbolType "delimiters")
install(FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType})
