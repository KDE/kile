file(GLOB symbols *.png)
set(symbolType "cyrillic")
install(FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/${symbolType})
