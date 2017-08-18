
########### install files ###############
file(GLOB kile_enc *.enc)
install(FILES ${kile_enc} DESTINATION ${KDE_INSTALL_DATAROOTDIR}/kile/encodings)
