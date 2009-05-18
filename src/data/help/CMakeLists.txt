
########### install files ###############
FILE(GLOB kile_lst *.lst)
INSTALL( FILES ${kile_lst} latexhelp.html DESTINATION ${DATA_INSTALL_DIR}/kile/help )
