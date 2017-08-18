file(GLOB symbols *.png)
set(symbolType "arrows")
install(FILES ${symbols} DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/mathsymbols/${symbolType})
