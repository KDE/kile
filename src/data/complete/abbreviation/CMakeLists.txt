FILE(GLOB kile_abbrevFiles *.cwl)
INSTALL( FILES ${kile_abbrevFiles} DESTINATION ${DATA_INSTALL_DIR}/kile/complete/abbreviation )
