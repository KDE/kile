
########### install files ###############
file(GLOB kile_template *.tex)
install(FILES ${kile_template} DESTINATION ${DATA_INSTALL_DIR}/kile/templates)
