/**************************************************************************************************
   Copyright (C) 2004-2005 by Holger Danielsson (holger.danielsson@t-online.de)
                 2004 by Jeroen Wijnhout
 **************************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dialogs/includegraphicsdialog.h"

#include <QRegExp>
#include <QFileInfo>
#include <QPixmap>

#include <KFileDialog>
#include <KIconLoader>
#include <KLineEdit>
#include <KLocale>
#include <KMessageBox>
#include <KProcess>
#include <KPushButton>

#include "errorhandler.h"
#include "kiledebug.h"
#include "kileconfig.h"
#include "kileinfo.h"
#include "editorextension.h"
#include "kileactions.h"
#include "kiletool_enums.h"

namespace KileDialog
{

IncludeGraphics::IncludeGraphics(QWidget *parent, const QString &startdir, KileInfo *ki) :
		KDialog(parent),
		m_startdir(startdir),
		m_width(0),
		m_height(0),
		m_ki(ki),
		m_proc(NULL)
{
	setCaption(i18n("Include Graphics"));
	setModal(true);
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	showButtonSeparator(true);

	QWidget *page = new QWidget(this);
	m_widget.setupUi(page);
	setMainWidget(page);

	// read configuration
	readConfig();

	slotChooseFilter();

	#if KDE_IS_VERSION(4,2,90)
		m_widget.edit_file->setStartDir(KUrl::fromPath(m_startdir));
	#else
		m_widget.edit_file->setUrl(KUrl::fromPath(m_startdir));
	#endif

	setFocusProxy(m_widget.edit_file);
	m_widget.edit_file->setFocus();

	connect(m_widget.cb_bb, SIGNAL(toggled(bool)),
	        this, SLOT(slotChooseFilter()));
	connect(m_widget.edit_file, SIGNAL(urlSelected(const KUrl&)),
	        this, SLOT(slotUrlSelected(const KUrl&)));
	connect(m_widget.edit_file, SIGNAL(textChanged(const QString&)),
	        this, SLOT(slotTextChanged(const QString&)));
	connect(m_widget.cb_figure, SIGNAL(toggled(bool)),
		this, SLOT(slotFigureSelected(bool)));
	connect(m_widget.cb_wrapfigure, SIGNAL(toggled(bool)),
		this, SLOT(slotWrapFigureSelected(bool)));
}

IncludeGraphics::~IncludeGraphics()
{
	delete m_proc;
}

////////////////////////////// configuration data //////////////////////////////

void IncludeGraphics::readConfig()
{
	m_widget.cb_center->setChecked(KileConfig::igCenter());
	m_widget.cb_bb->setChecked(KileConfig::igBoundingBox());
	m_widget.cb_graphicspath->setChecked(KileConfig::igGraphicspath());

	m_widget.cb_figure->setChecked(KileConfig::igFigure());
	m_widget.cb_Bottom->setChecked(KileConfig::igBottom());
	m_widget.cb_Force->setChecked(KileConfig::igForce());
	m_widget.cb_Here->setChecked(KileConfig::igHere());
	m_widget.cb_Page->setChecked(KileConfig::igPage());
	m_widget.cb_Top->setChecked(KileConfig::igTop());

	m_widget.cb_wrapfigure->setChecked(KileConfig::igWrapFigure());
	m_widget.cb_wrapright->setChecked(KileConfig::igWrapRight());
	m_widget.cb_wrapleft->setChecked(KileConfig::igWrapLeft());
	m_widget.cb_wrapinside->setChecked(KileConfig::igWrapInside());
	m_widget.cb_wrapoutside->setChecked(KileConfig::igWrapOutside());
	m_widget.cb_wrapfloat->setChecked(KileConfig::igWrapFloat());

	m_imagemagick = KileConfig::imagemagick();
	m_boundingbox = KileConfig::boundingbox();
	m_defaultresolution = KileConfig::resolution();
}

void IncludeGraphics::writeConfig()
{
	KileConfig::setIgCenter(m_widget.cb_center->isChecked());
	KileConfig::setIgBoundingBox(m_widget.cb_bb->isChecked());
	KileConfig::setIgGraphicspath(m_widget.cb_graphicspath->isChecked());

	KileConfig::setIgFigure(m_widget.cb_figure->isChecked());
	KileConfig::setIgBottom(m_widget.cb_Bottom->isChecked());
	KileConfig::setIgHere(m_widget.cb_Here->isChecked());
	KileConfig::setIgPage(m_widget.cb_Page->isChecked());
	KileConfig::setIgTop(m_widget.cb_Top->isChecked());
	KileConfig::setIgForce(m_widget.cb_Force->isChecked());

	KileConfig::setIgWrapFigure(m_widget.cb_wrapfigure->isChecked());
	KileConfig::setIgWrapRight(m_widget.cb_wrapright->isChecked());
	KileConfig::setIgWrapLeft(m_widget.cb_wrapleft->isChecked());
	KileConfig::setIgWrapInside(m_widget.cb_wrapinside->isChecked());
	KileConfig::setIgWrapOutside(m_widget.cb_wrapoutside->isChecked());
	KileConfig::setIgWrapFloat(m_widget.cb_wrapfloat->isChecked());
}

////////////////////////////// determine the whole tag //////////////////////////////

QString IncludeGraphics::getTemplate()
{
	QString s;

	// state of figure, wrapfigure, and center checkbox
	bool figure = m_widget.cb_figure->isChecked();
	bool wrapfigure = m_widget.cb_wrapfigure->isChecked();
	bool center = m_widget.cb_center->isChecked();
	const QString indent = (figure || center) ? m_ki->editorExtension()->autoIndentEnvironment() : QString();

	// build tags for start of figure environment
	if (figure) {
		// positioning for figure environment
		QString p;
		bool here 	= m_widget.cb_Here->isChecked();
		bool top 	= m_widget.cb_Top->isChecked();
		bool bottom 	= m_widget.cb_Bottom->isChecked();
		bool page 	= m_widget.cb_Page->isChecked();
		bool force 	= m_widget.cb_Force->isChecked();
		bool custom 	= m_widget.cb_custom->isChecked();

		// build position string
		if (here||top||bottom||page||custom) { // Don't check for force -- if it is the only selection, just skip the position tag
			p += '[';
			if (here)	p+= 'h';
			if (top)	p+= 't';
			if (bottom)	p+= 'b';
			if (page)	p+= 'p';
			if (force)    	p+= '!';
			if (custom)	p+= m_widget.edit_custom->text();
			p += ']';
		}


		// add start of figure environment
		s += "\\begin{figure}" + p + '\n';
	}

	// build tags for start of wrapfigure environment
	if (wrapfigure) {

		s += "\\begin{wrapfigure}";

		// number of lines in length
		if (!m_widget.edit_wraplines->text().isEmpty()) {
			s += '[' + m_widget.edit_wraplines->text() + ']';
		}

		// positioning for wrapfigure environment
		bool wrapfloat;
 		wrapfloat = m_widget.cb_wrapfloat->isChecked();
 		if (m_widget.cb_wrapright->isChecked()){
			if (wrapfloat) 	s += "{R}";
			else		s += "{r}";
		}
		if (m_widget.cb_wrapleft->isChecked()){
			if (wrapfloat) 	s += "{L}";
			else		s += "{l}";
		}
		if (m_widget.cb_wrapinside->isChecked()){
			if (wrapfloat) 	s += "{I}";
			else		s += "{i}";
		}
		if (m_widget.cb_wrapoutside->isChecked()){
			if (wrapfloat) 	s += "{O}";
			else		s += "{i}";
		}

		// overhang into margin
		if (!m_widget.edit_wrapoverhang->text().isEmpty()) {
			s += '[' + m_widget.edit_wrapoverhang->text() + ']';
		}

		// width of figure
		if (!m_widget.edit_wrapwidth->text().isEmpty()) {
			s += '{' + m_widget.edit_wrapwidth->text() + '}';
		}

		// end of wrapfigure options
		s += '\n';

		// Include warning in comment if wrapfig is not loaded.
		// Sending a warning to the log here would be good, but
		// the log seems to get cleared before user could catch
		// the warning.
		QStringList packagelist = m_ki->allPackages();
		if (!packagelist.contains("wrapfig")) {
			s += "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";
			s += "%%% You will need to add \\usepackage{wrapfig} to your preamble to use textwrapping %%%\n";
			s += "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n";
		}
	}


	// add start of center environment ?
	if (center) {
		if (figure || wrapfigure) {
			s += indent + "\\centering\n";
		}
		else {
			s += "\\begin{center}\n";
		}
	}

	// add includegraphics command
	s += indent + "\\includegraphics";

	// add some options
	QString options = getOptions();
	if (!options.isEmpty()) {
		s += '[' + options + ']';
	}

	// add name of picture
	// either take the filename or try to take the relative part of the name
	QString filename = (m_widget.cb_graphicspath->isChecked())
			 ? QFileInfo(m_widget.edit_file->lineEdit()->text()).fileName()
			 : KUrl::relativePath(QFileInfo(m_ki->getCompileName()).path(), m_widget.edit_file->lineEdit()->text());
	s += '{' + filename + "}\n";

	// add some comments (depending of given resolution, this may be wrong!)
	QString info = getInfo();
	if (info.length() > 0) {
		s += indent + info + '\n';
	}

	// close center environment ?
	if (center && !figure && !wrapfigure) {
		s += "\\end{center}\n";
	}

	// close figure environment ?
	if (figure)
	{
		QString caption = m_widget.edit_caption->text().trimmed();
		if (!caption.isEmpty()) {
			s +=  indent + "\\caption{" + caption + "}\n";
		}
		QString label = m_widget.edit_label->text().trimmed();
		if (!label.isEmpty() && label != "fig:") {
			s +=  indent + "\\label{" + label + "}\n";
		}
		s += "\\end{figure}\n";
	}

	if (wrapfigure)
	{
		QString caption = m_widget.edit_wrapcaption->text().trimmed();
		if (!caption.isEmpty()) {
			s +=  indent + "\\caption{" + caption + "}\n";
		}
		QString label = m_widget.edit_wraplabel->text().trimmed();
		if (!label.isEmpty() && label != "fig:") {
			s +=  indent + "\\label{" + label + "}\n";
		}
		s += "\\end{wrapfigure}\n";
	}

	return s;
}

QString IncludeGraphics::getFilename()
{
	return m_widget.edit_file->lineEdit()->text();
}

////////////////////////////// some calculations //////////////////////////////

QString IncludeGraphics::getOptions()
{
	QString s = "";

	if (! m_widget.edit_width->text().isEmpty()) {
		s += ",width=" + m_widget.edit_width->text();
	}

	if (! m_widget.edit_height->text().isEmpty()) {
		s += ",height=" + m_widget.edit_height->text();
	}

	if (! m_widget.edit_angle->text().isEmpty()) {
		s += ",angle=" + m_widget.edit_angle->text();
	}

	// Only dvips needs the bounding box, not pdftex/pdflatex.
	// But it will be always inserted as a comment.
	if (!m_widget.edit_bb->text().isEmpty() && m_widget.cb_bb->isChecked()) {
		s += ",bb=" + m_widget.edit_bb->text();
	}

	if (!m_widget.edit_scale->text().isEmpty()) {
		s += ",scale=" + m_widget.edit_scale->text();
	}

	if (m_widget.cb_keepAspect->isChecked()) {
		s+= ",keepaspectratio=true";
	}

	if (m_widget.cb_clip->isChecked()) {
		QString l="0pt", b="0pt", r="0pt", t="0pt";
		if (!m_widget.edit_trimLeft->text().isEmpty()) {
			l = m_widget.edit_trimLeft->text();
		}
		if (!m_widget.edit_trimBottom->text().isEmpty()) {
			b = m_widget.edit_trimBottom->text();
		}
		if (!m_widget.edit_trimRight->text().isEmpty()) {
			r = m_widget.edit_trimRight->text();
		}
		if (!m_widget.edit_trimTop->text().isEmpty()) {
			t = m_widget.edit_trimTop->text();
		}
		s += ",clip=true,trim=" + l + ' ' + b + ' ' + r + ' ' + t;
	}

	if (s.left(1) == ",") {
		return s.right(s.length() - 1);
	}
	else {
		return s;
	}
}

////////////////////////////// graphics info //////////////////////////////

QString IncludeGraphics::getInfo()
{
	QString wcm,hcm,dpi;
	int wpx=0,hpx=0;

	bool ok = getPictureSize(wpx, hpx, dpi, wcm, hcm);
	if (! ok)
		return "";
	else
	{
		QFileInfo fi(m_widget.edit_file->lineEdit()->text());

		return "% " + fi.baseName() + '.' + fi.completeSuffix()
					 + QString(": %1x%2 pixel").arg(wpx).arg(hpx)
					 + ", " + dpi + "dpi"
					 + ", " + wcm + 'x' + hcm + " cm"
					 + ", bb=" + m_widget.edit_bb->text();
	}
}

void IncludeGraphics::setInfo()
{
	QString text;
	QString wcm, hcm, dpi;
	int wpx, hpx;

	if (!m_widget.edit_file->lineEdit()->text().isEmpty() && getPictureSize(wpx, hpx, dpi, wcm, hcm))
	{
		text = QString("%1x%2 pixel").arg(wpx).arg(hpx)
					 + " / " + wcm + 'x' + hcm + " cm"
					 + "  (" + dpi + "dpi)";
	}
	else
		text = "---";

// insert text
	m_widget.infolabel->setText(text);
}

bool IncludeGraphics::getPictureSize(int &wpx, int &hpx, QString &dpi, QString &wcm, QString &hcm)
{
	wpx = m_width;
	hpx = m_height;

	dpi = QString::number((int)(m_resolution + 0.5));

	// convert from inch to cm
	float w = (float)m_width / m_resolution * 2.54;
	wcm = wcm.setNum(w, 'f', 2);

	float h = (float)m_height / m_resolution * 2.54;
	hcm = hcm.setNum(h, 'f', 2);
	return true;
}

void IncludeGraphics::slotChooseFilter()
{
	QString filter = (!m_widget.cb_bb->isChecked())
			? i18n("*.png *.jpg *.pdf|Graphics\n")              // dani  31.7.2004
			+ "*.png|PNG Files\n"
			+ "*.jpg|JPG Files\n"
			+ "*.pdf|PDF Files\n"
			+ "*|All Files"
	: i18n("*.png *.jpg *.eps.gz *.eps|Graphics\n")     // dani  31.7.2004
			+ "*.png|PNG Files\n"
			+ "*.jpg|JPG Files\n"
			+ "*.eps.gz|Zipped EPS Files\n"
			+ "*.eps|EPS Files\n"
			+ "*|All Files";
	m_widget.edit_file->setFilter(filter);
}

void IncludeGraphics::slotUrlSelected(const KUrl& url)
{
	QFileInfo fi(url.toLocalFile());

	// could we accept the picture?
	if (!url.toLocalFile().isEmpty() && fi.exists() && fi.isReadable())
	{
		// execute the command and filter the result:
		// eps|eps.gz --> %%BoundingBox: 0 0 123 456
		// bitmaps    --> w=123 h=456 dpi=789
		QString grep = " | grep -m1 \"^%%BoundingBox:\"";
		QString ext = fi.completeSuffix();
		if (ext == "eps"){
			execute("cat " + url.toLocalFile() + grep);
		}
		else if (ext == "eps.gz"){
				execute("gunzip -c " + url.toLocalFile() + grep);
		}
		else{
			execute("identify -format \"w=%w h=%h dpi=%x\" \"" + url.toLocalFile() + "\"");
		}
	} else {
		KILE_DEBUG() << "=== IncludeGraphics::error ====================";
		KILE_DEBUG() << "   filename: '" << url.toLocalFile() << "'";

		m_widget.infolabel->setText("---");
		m_widget.edit_bb->setText("");
	}
}

void IncludeGraphics::slotTextChanged(const QString& string)
{
	slotUrlSelected(KUrl(string));
}

void IncludeGraphics::execute(const QString &command)
{
	if (!m_boundingbox || (!m_imagemagick && command.left(8) == "identify"))
		return;

	if (m_proc)
		delete m_proc;

	m_proc = new KProcess(this);
	m_proc->setShellCommand(command);
	m_proc->setOutputChannelMode(KProcess::MergedChannels);
	m_proc->setReadChannel(QProcess::StandardOutput);

	connect(m_proc, SIGNAL(readyReadStandardOutput()),
					this, SLOT(slotProcessOutput()));
	connect(m_proc, SIGNAL(readyReadStandardError()),
					this, SLOT(slotProcessOutput()));
	connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)),
					this, SLOT(slotProcessExited(int, QProcess::ExitStatus)));

	m_output = "";
	KILE_DEBUG() << "=== IncludeGraphics::execute ====================";
	KILE_DEBUG() << "   execute '" << command << "'";

	m_proc->start();
}

// get all output of identify

void IncludeGraphics::slotProcessOutput()
{
	m_output += m_proc->readAll();
}

// identify was called

void IncludeGraphics::slotProcessExited(int /* exitCode */, QProcess::ExitStatus exitStatus)
{
	if (exitStatus == QProcess::NormalExit) {
		KILE_DEBUG() << "   result: " << m_output;

		// set the default resolution
		m_resolution = m_defaultresolution;

		// analyze the result
		if (m_output.left(14) == "%%BoundingBox:") {
			m_widget.edit_bb->setText(m_output.trimmed().mid(15, m_output.length() - 15));

			// show information
			setInfo();
		}
		else {
			if (m_output.left(2) == "w=") {
				// dani  31.7.2004
				// older version of imagemagick (pre 6.0):
				//  - doesn't care of PixelsPerCentimeter, but always works with PixelsPerInch
				//  - doesn't use floating numbers as resolution
				// so the bounding box has to be calculated in a different way

				// this regexp will accept floating point numbers as resolution
				QRegExp reg("w=(\\d+)\\s+h=(\\d+)\\s+dpi=([0-9.]+) (.*)");
				if(reg.indexIn(m_output) == -1) {
					return;
				}

				// get bounding box and resolution
				bool ok;
				m_width = (int)reg.cap(1).toInt(&ok);
				if (!ok)
					return;

				m_height = (int)reg.cap(2).toInt(&ok);
				if (!ok)
					return;

				float res = (float)reg.cap(3).toFloat(&ok);
				if (!ok)
					return;
				if (res > 0.0)
					m_resolution = res;

				// look, if resolution is in PixelsPerCentimeter
				if (reg.cap(4).trimmed() == "PixelsPerCentimeter")
					m_resolution *= 2.54;

				// calc the bounding box
				int bbw = (int)((float)m_width * 72.0 / m_resolution + 0.5);
				int bbh = (int)((float)m_height * 72.0 / m_resolution + 0.5);

				// take width and height as parameters for the bounding box
				m_widget.edit_bb->setText(QString("0 0 ") + QString::number(bbw)
																	+ ' '
																	+ QString::number(bbh)
																 );

				// show information
				setInfo();

			}
		}
	}
}

void IncludeGraphics::slotButtonClicked(int button)
{
	if(button == KDialog::Ok){
		if(checkParameter()){
			writeConfig();
			accept();
		}
	}
	else{
		KDialog::slotButtonClicked(button);
	}
}

void IncludeGraphics::slotWrapFigureSelected(bool state) {
	if (m_widget.cb_figure->isChecked() && state) {
		m_widget.cb_figure->setChecked(false);
	}
	// Adds warning to log if wrapfig isn't in the preamble
	QStringList packagelist = m_ki->allPackages();
	if (!packagelist.contains("wrapfig")) {
		m_ki->errorHandler()->printMessage(KileTool::Error, i18n("You must include the wrapfig package to use the text wrapping options"), i18n("Missing Package"));
	}
}

void IncludeGraphics::slotFigureSelected(bool state) {
	if (m_widget.cb_wrapfigure->isChecked() && state) {
		m_widget.cb_wrapfigure->setChecked(false);
	}
}

bool IncludeGraphics::checkParameter()
{
	QString filename = m_widget.edit_file->lineEdit()->text().trimmed();
	m_widget.edit_file->lineEdit()->setText(filename);

	if (filename.isEmpty())
	{
		if (KMessageBox::warningYesNo(this, i18n("No graphics file was given. Proceed any way?")) == KMessageBox::No)
			return false;
	}
	else
	{
		QFileInfo fi(filename);
		if (! fi.exists())
		{
			if (KMessageBox::warningYesNo(this, i18n("The graphics file does not exist. Proceed any way?")) == KMessageBox::No)
				return false;
		}
	}

	return true;
}

}

#include "includegraphicsdialog.moc"
