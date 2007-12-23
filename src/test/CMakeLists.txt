
########### install files ###############

SET( testFiles
	runTests.sh
	test.tex
	test_plain.tex
	test.bib
	test_bib.tex
	test_index.tex
)

INSTALL( FILES ${testFiles} DESTINATION ${DATA_INSTALL_DIR}/kile/test )
