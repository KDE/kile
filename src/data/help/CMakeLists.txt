
########### install files ###############
file(GLOB kile_lst *.lst)
install(FILES ${kile_lst} latexhelp.html latex2e-texlive.html DESTINATION ${DATA_INSTALL_DIR}/kile/help)
