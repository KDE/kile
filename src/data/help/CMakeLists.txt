
########### install files ###############
FILE(GLOB kile_lst *.lst)
INSTALL( FILES ${kile_lst} latexhelp.html latex2e-texlive.html DESTINATION ${DATA_INSTALL_DIR}/kile/help )
