
########### install files ###############
file(GLOB kile_jsplug *.js)
install(FILES ${kile_jsplug} DESTINATION ${DATA_INSTALL_DIR}/kile/script-plugins)
