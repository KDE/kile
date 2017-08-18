file(GLOB kile_dictFiles *.cwl)
install(FILES ${kile_dictFiles} DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/complete/dictionary)
