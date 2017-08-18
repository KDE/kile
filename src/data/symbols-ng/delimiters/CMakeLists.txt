file(GLOB symbols *.png)
set(symbolType "delimiters")
install(FILES ${symbols} DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/mathsymbols/${symbolType})
