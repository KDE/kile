####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = template_Article.tex template_Book.tex template_Letter.tex template_Report.tex 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kile/templates/
	$(INSTALL_DATA) $(srcdir)/template_Article.tex $(kde_datadir)/kile/templates/template_Article.tex
	$(mkinstalldirs) $(kde_datadir)/kile/templates/
	$(INSTALL_DATA) $(srcdir)/template_Book.tex $(kde_datadir)/kile/templates/template_Book.tex
	$(mkinstalldirs) $(kde_datadir)/kile/templates/
	$(INSTALL_DATA) $(srcdir)/template_Letter.tex $(kde_datadir)/kile/templates/template_Letter.tex
	$(mkinstalldirs) $(kde_datadir)/kile/templates/
	$(INSTALL_DATA) $(srcdir)/template_Report.tex $(kde_datadir)/kile/templates/template_Report.tex

uninstall-local:
	-rm -f $(kde_datadir)/kile/templates/template_Article.tex
	-rm -f $(kde_datadir)/kile/templates/template_Book.tex
	-rm -f $(kde_datadir)/kile/templates/template_Letter.tex
	-rm -f $(kde_datadir)/kile/templates/template_Report.tex

####### kdevelop will overwrite this part!!! (end)############
