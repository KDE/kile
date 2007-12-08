
########### install files ###############
file(GLOB symbols *.png)

INSTALL( FILES ${symbols} DESTINATION ${DATA_INSTALL_DIR}/kile/mathsymbols/relation )
