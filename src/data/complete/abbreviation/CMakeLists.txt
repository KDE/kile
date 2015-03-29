file(GLOB kile_abbrevFiles *.cwl)
install(FILES ${kile_abbrevFiles} DESTINATION ${DATA_INSTALL_DIR}/kile/complete/abbreviation)
