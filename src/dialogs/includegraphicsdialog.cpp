/***************************************************************************
    date                 : Dec 06 2005
    version              : 0.24
    copyright            : (C) 2004-2005 by Holger Danielsson, 2004 Jeroen Wijnhout
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

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

#include "kiledebug.h"
#include "kileconfig.h"
#include "kileinfo.h"
#include "kileedit.h"

namespace KileDialog
{

IncludeGraphics::IncludeGraphics(QWidget *parent, const QString &startdir, KileInfo *ki) :
		KDialog(parent),
		m_startdir(startdir),
		m_ki(ki),
		m_proc(0)
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
	setFocusProxy(m_widget.edit_file);
	m_widget.edit_file->setFocus();

	connect(m_widget.cb_pdftex, SIGNAL(toggled(bool)),
	        this, SLOT(slotChooseFilter()));
	connect(m_widget.edit_file, SIGNAL(urlSelected(const KUrl&)),
	        this, SLOT(slotUrlSelected(const KUrl&)));
	connect(m_widget.edit_file, SIGNAL(textChanged(const QString&)),
	        this, SLOT(slotTextChanged(const QString&)));
}

IncludeGraphics::~IncludeGraphics()
{
	delete m_proc;
}

////////////////////////////// configuration data //////////////////////////////

void IncludeGraphics::readConfig()
{
	m_widget.cb_center->setChecked(KileConfig::igCenter());
	m_widget.cb_pdftex->setChecked(KileConfig::igPdftex());
	m_widget.cb_graphicspath->setChecked(KileConfig::igGraphicspath());
	m_widget.cb_figure->setChecked(KileConfig::igFigure());

	m_imagemagick = KileConfig::imagemagick();
	m_boundingbox = KileConfig::boundingbox();
	m_defaultresolution = KileConfig::resolution();
}

void IncludeGraphics::writeConfig()
{
	KileConfig::setIgCenter(m_widget.cb_center->isChecked());
	KileConfig::setIgPdftex(m_widget.cb_pdftex->isChecked());
	KileConfig::setIgGraphicspath(m_widget.cb_graphicspath->isChecked());
	KileConfig::setIgFigure(m_widget.cb_figure->isChecked());
}

////////////////////////////// determine the whole tag //////////////////////////////

QString IncludeGraphics::getTemplate()
{
	QString s;

	// state of figure and center checkbox
	bool figure = m_widget.cb_figure->isChecked();
	bool center = m_widget.cb_center->isChecked();
	QString indent = (figure || center) ? m_ki->editorExtension()->autoIndentEnvironment() : QString::null;

	// add start of figure environment ?
	if (figure)
		s += "\\begin{figure}\n";

	// add start of center environment ?
	if (center)
	{
		if (figure)
			s += indent + "\\centering\n";
		else
			s += "\\begin{center}\n";
	}

	// add includegraphics command
	s += indent + "\\includegraphics";

	// add some options
	QString options = getOptions();
	if (!options.isEmpty())
		s += '[' + options + ']';

	// add name of picture
	// either take the filename or try to take the relative part of the name
	QString filename = (m_widget.cb_graphicspath->isChecked())
										 ? QFileInfo(m_widget.edit_file->lineEdit()->text()).fileName()
										 : m_ki->relativePath(QFileInfo(m_ki->getCompileName()).path(), m_widget.edit_file->lineEdit()->text());
	s += '{' + filename + "}\n";

	// add some comments (depending of given resolution, this may be wrong!)
	QString info = getInfo();
	if (info.length() > 0)
		s += indent + info + '\n';

	// close center environment ?
	if (center && !figure)
		s += "\\end{center}\n";

	// close figure environment ?
	if (figure)
	{
		QString caption = m_widget.edit_caption->text().trimmed();
		if (!caption.isEmpty())
			s +=  indent + "\\caption{" + caption + "}\n";
		QString label = m_widget.edit_label->text().trimmed();
		if (!label.isEmpty() && label != "fig:")
			s +=  indent + "\\label{" + label + "}\n";
		s += "\\end{figure}\n";
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

	if (! m_widget.edit_width->text().isEmpty())
		s += ",width=" + m_widget.edit_width->text();

	if (! m_widget.edit_height->text().isEmpty())
		s += ",height=" + m_widget.edit_height->text();

	if (! m_widget.edit_angle->text().isEmpty())
		s += ",angle=" + m_widget.edit_angle->text();

	// Only dvips needs the bounding box, not pdftex/pdflatex.
	// But it will be always inserted as a comment.
	if (!m_widget.edit_bb->text().isEmpty() && !m_widget.cb_pdftex->isChecked())
		s += ",bb=" + m_widget.edit_bb->text();

	if (s.left(1) == ",")
		return s.right(s.length() - 1);
	else
		return s;
}

////////////////////////////// graphics info //////////////////////////////

QString IncludeGraphics::getInfo()
{
	QString wcm, hcm, dpi;
	int wpx, hpx;

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
	QString filter = (m_widget.cb_pdftex->isChecked())
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
	QFileInfo fi(url.path());

	// could we accept the picture?
	if (!url.path().isEmpty() && fi.exists() && fi.isReadable())
	{
		// execute the command and filter the result:
		// eps|eps.gz --> %%BoundingBox: 0 0 123 456
		// bitmaps    --> w=123 h=456 dpi=789
		QString grep = " | grep -m1 \"^%%BoundingBox:\"";
		QString ext = fi.completeSuffix();
		if (ext == "eps")
			execute("cat " + url.path() + grep);
		else
			if (ext == "eps.gz")
				execute("gunzip -c " + url.path() + grep);
		else
			execute("identify -format \"w=%w h=%h dpi=%x\" " + url.path());
	} else {
		KILE_DEBUG() << "=== IncludeGraphics::error ====================" << endl;
		KILE_DEBUG() << "   filename: '" << url.path() << "'" << endl;

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
	KILE_DEBUG() << "=== IncludeGraphics::execute ====================" << endl;
	KILE_DEBUG() << "   execute '" << command << "'" << endl;

	m_proc->start();
}

// get all output of identify

void IncludeGraphics::slotProcessOutput()
{
	m_output += m_proc->readAll();
}

// identify was called

void IncludeGraphics::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
	if (exitStatus == QProcess::NormalExit) {
		KILE_DEBUG() << "   result: " << m_output << endl;

		// set the default resolution
		m_resolution = m_defaultresolution;

		// analyze the result
		if (m_output.left(14) == "%%BoundingBox:")
		{
			m_widget.edit_bb->setText(m_output.trimmed().mid(15, m_output.length() - 15));

			// show information
			setInfo();
		}
		else
			if (m_output.left(2) == "w=")
			{
				// dani  31.7.2004
				// older version of imagemagick (pre 6.0):
				//  - doesn't care of PixelsPerCentimeter, but always works with PixelsPerInch
				//  - doesn't use floating numbers as resolution
				// so the bounding box has to be calculated in a different way

				// this regexp will accept floating point numbers as resolution
				QRegExp reg("w=(\\d+)\\s+h=(\\d+)\\s+dpi=([0-9.]+) (.*)");
				if (reg.search(m_output) == -1)
					return;

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

void IncludeGraphics::slotOk()
{
	if (checkParameter())  {
		writeConfig();
		accept();
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
