file(GLOB symbols *.png)
set(symbolType "relation")
install(FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType})
