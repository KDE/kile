
########### install files ###############
file(GLOB kile_index *.index)
install(FILES ${kile_index} latexhelp.html DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/help)
install(DIRECTORY "unofficial-latex2e-reference-manual" DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/help)
