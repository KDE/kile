file(GLOB symbols *.png)
set(symbolType "misc-text")
install(FILES ${symbols} DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/mathsymbols/${symbolType})
