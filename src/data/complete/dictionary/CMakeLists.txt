file(GLOB kile_dictFiles *.cwl)
install(FILES ${kile_dictFiles} DESTINATION ${DATA_INSTALL_DIR}/kile/complete/dictionary)
