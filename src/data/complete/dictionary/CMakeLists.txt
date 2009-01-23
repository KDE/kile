FILE(GLOB kile_dictFiles *.cwl)
INSTALL( FILES ${kile_dictFiles} DESTINATION ${DATA_INSTALL_DIR}/kile/complete/dictionary )
