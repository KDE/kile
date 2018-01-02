/**************************************************************************************************
   Copyright (C) 2004-2005  Holger Danielsson <holger.danielsson@t-online.de>
                 2004       Jeroen Wijnhout
                 2015       Andreas Cord-Landwehr <cordlandwehr@kde.org>
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
#include "editorextension.h"
#include "errorhandler.h"
#include "kileactions.h"
#include "kileconfig.h"
#include "kiledebug.h"
#include "kileinfo.h"
#include "kiletool_enums.h"

#include <KConfigGroup>
#include <KLineEdit>
#include <KLocalizedString>
#include <KProcess>

#include <QDialogButtonBox>
#include <QFileInfo>
#include <QLineEdit>
#include <QPushButton>
#include <QRegExp>
#include <QVBoxLayout>

namespace KileDialog
{

IncludeGraphics::IncludeGraphics(QWidget *parent, const QString &startdir, KileInfo *ki)
    : QDialog(parent)
    , m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel))
    , m_startdir(startdir)
    , m_width(0)
    , m_height(0)
    , m_ki(ki)
    , m_proc(Q_NULLPTR)
{
    setWindowTitle(i18n("Include Graphics"));
    setModal(true);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    QWidget *page = new QWidget(this);
    m_widget.setupUi(page);
    mainLayout->addWidget(page);

    // read configuration
    readConfig();
    onChooseFilter();

    setFocusProxy(m_widget.edit_file);
    m_widget.edit_file->setMode(KFile::File | KFile::LocalOnly);
    m_widget.edit_file->setStartDir(QUrl::fromLocalFile(m_startdir));
    m_widget.edit_file->setFocus();

    connect(m_widget.cb_bb, &QCheckBox::toggled, this, &IncludeGraphics::onChooseFilter);
    connect(m_widget.edit_file, &KUrlRequester::urlSelected, this, &IncludeGraphics::onUrlSelected);
    connect(m_widget.edit_file, &KUrlRequester::textChanged, this, &IncludeGraphics::onTextChanged);
    connect(m_widget.cb_figure, &QGroupBox::toggled, this, &IncludeGraphics::onFigureSelected);
    connect(m_widget.cb_wrapfigure, &QGroupBox::toggled, this, &IncludeGraphics::onWrapFigureSelected);

    QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setEnabled(false);
    mainLayout->addWidget(m_buttonBox);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(this, &QDialog::accepted, this, &IncludeGraphics::onAccepted);
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
        if (m_widget.cb_wrapright->isChecked()) {
            if (wrapfloat) 	s += "{R}";
            else		s += "{r}";
        }
        if (m_widget.cb_wrapleft->isChecked()) {
            if (wrapfloat) 	s += "{L}";
            else		s += "{l}";
        }
        if (m_widget.cb_wrapinside->isChecked()) {
            if (wrapfloat) 	s += "{I}";
            else		s += "{i}";
        }
        if (m_widget.cb_wrapoutside->isChecked()) {
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
        } else {
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
    const QString relativeUrl = QDir(QFileInfo(m_ki->getCompileName()).path()).relativeFilePath(m_widget.edit_file->lineEdit()->text());
    QString filename = (m_widget.cb_graphicspath->isChecked())
                       ? QFileInfo(m_widget.edit_file->lineEdit()->text()).fileName()
                       : relativeUrl;
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
    if (figure) {
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

    if (wrapfigure) {
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
    QString s = QString();

    if (!m_widget.edit_width->text().isEmpty()) {
        s += ",width=" + m_widget.edit_width->text();
    }

    if (!m_widget.edit_height->text().isEmpty()) {
        s += ",height=" + m_widget.edit_height->text();
    }

    if (!m_widget.edit_angle->text().isEmpty()) {
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
    } else {
        return s;
    }
}

////////////////////////////// graphics info //////////////////////////////

QString IncludeGraphics::getInfo()
{
    QString wcm, hcm, dpi;
    int wpx = 0, hpx = 0;

    bool ok = getPictureSize(wpx, hpx, dpi, wcm, hcm);
    if (!ok) {
        return QString();
    } else {
        QFileInfo fi(m_widget.edit_file->lineEdit()->text());

        return "% " + fi.baseName() + '.' + fi.completeSuffix()
               + QString(": %1x%2 px").arg(wpx).arg(hpx)
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

    if (!m_widget.edit_file->lineEdit()->text().isEmpty() && getPictureSize(wpx, hpx, dpi, wcm, hcm)) {
        text = QString("%1x%2 px").arg(wpx).arg(hpx)
               + " / " + wcm + 'x' + hcm + " cm"
               + "  (" + dpi + "dpi)";
    } else {
        text = "---";
    }

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

void IncludeGraphics::onChooseFilter()
{
    QString filter = (!m_widget.cb_bb->isChecked())
                     ? i18n("*.png *.jpg *.pdf *.ps *.eps|Graphics\n")
                     + "*.png|PNG Files\n"
                     + "*.jpg|JPG Files\n"
                     + "*.pdf|PDF Files\n"
                     + "*.eps *ps|Postscript Files\n"
                     + "*|All Files"
                     : i18n("*.png *.jpg *.eps.gz *.eps|Graphics\n")
                     + "*.png|PNG Files\n"
                     + "*.jpg|JPG Files\n"
                     + "*.eps.gz|Zipped EPS Files\n"
                     + "*.eps|EPS Files\n"
                     + "*|All Files";
    m_widget.edit_file->setFilter(filter);
}

void IncludeGraphics::onUrlSelected(const QUrl &url)
{
    QFileInfo fi(url.toLocalFile());

    // could we accept the picture?
    if (!url.toLocalFile().isEmpty() && fi.exists() && fi.isReadable()) {
        // execute the command and filter the result:
        // eps|eps.gz --> %%BoundingBox: 0 0 123 456
        // bitmaps    --> w=123 h=456 dpi=789
        QString grep = " | grep -m1 \"^%%BoundingBox:\"";
        QString ext = fi.completeSuffix();
        if (ext == "eps") {
            execute("cat " + url.toLocalFile() + grep);
        }
        else if (ext == "eps.gz") {
            execute("gunzip -c " + url.toLocalFile() + grep);
        }
        else {
            execute("identify -format \"w=%w h=%h dpi=%x %U\" \"" + url.toLocalFile() + "\"");
        }
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    } else {
        KILE_DEBUG_MAIN << "=== IncludeGraphics::error ====================";
        KILE_DEBUG_MAIN << "   filename: '" << url.toLocalFile() << "'";

        m_widget.infolabel->setText("---");
        m_widget.edit_bb->setText("");
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

void IncludeGraphics::onTextChanged(const QString &string)
{
    onUrlSelected(QUrl::fromLocalFile(string.trimmed()));
}

void IncludeGraphics::execute(const QString &command)
{
    if (!m_boundingbox || (!m_imagemagick && command.left(8) == "identify")) {
        return;
    }

    if (m_proc) {
        delete m_proc;
    }

    m_proc = new KProcess(this);
    m_proc->setShellCommand(command);
    m_proc->setOutputChannelMode(KProcess::MergedChannels);
    m_proc->setReadChannel(QProcess::StandardOutput);

    connect(m_proc, &KProcess::readyReadStandardOutput, this, &IncludeGraphics::onProcessOutput);
    connect(m_proc, &KProcess::readyReadStandardError, this, &IncludeGraphics::onProcessOutput);
    connect(m_proc, static_cast<void (KProcess::*)(int, QProcess::ExitStatus)>(&KProcess::finished), this, &IncludeGraphics::onProcessExited);

    m_output = QString();
    KILE_DEBUG_MAIN << "=== IncludeGraphics::execute ====================";
    KILE_DEBUG_MAIN << "   execute '" << command << "'";

    m_proc->start();
}

// get all output of identify

void IncludeGraphics::onProcessOutput()
{
    m_output += m_proc->readAll();
}

// identify was called

void IncludeGraphics::onProcessExited(int /* exitCode */, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {
        KILE_DEBUG_MAIN << "   result: " << m_output;

        // set the default resolution
        m_resolution = m_defaultresolution;

        // analyze the result
        if (m_output.left(14) == "%%BoundingBox:") {
            m_widget.edit_bb->setText(m_output.trimmed().mid(15, m_output.length() - 15));

            // this regexp will extract width and height from the bounding box
            QRegExp reg("(\\d+) (\\d+) (\\d+) (\\d+)");
            if(reg.indexIn(m_output) == -1) {
                return;
            }

            // determine lower-left-x (llx), lower-left-y (lly), upper-right-x (urx) and upper-right-y (ury)
            bool ok;
            int llx = (int)reg.cap(1).toInt(&ok);
            if (!ok) {
                return;
            }

            int lly = (int)reg.cap(2).toInt(&ok);
            if (!ok) {
                return;
            }

            int urx = (int)reg.cap(3).toInt(&ok);
            if (!ok) {
                return;
            }

            int ury = (int)reg.cap(4).toInt(&ok);
            if (!ok) {
                return;
            }

            // calculate width and height from 72 dpi of eps graphics to the default resolution
            m_width = ((urx-llx)*m_resolution) / 72;
            m_height = ((ury-lly)*m_resolution) / 72;

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
                if (!ok) {
                    return;
                }

                m_height = (int)reg.cap(2).toInt(&ok);
                if (!ok) {
                    return;
                }

                float res = (float)reg.cap(3).toFloat(&ok);
                if (!ok) {
                    return;
                }
                if (res > 0.0) {
                    m_resolution = res;
                }

                // look, if resolution is in PixelsPerCentimeter
                if (reg.cap(4).trimmed() == "PixelsPerCentimeter") {
                    m_resolution *= 2.54;
                }

                // calc the bounding box
                int bbw = (int)((float)m_width * 72.0 / m_resolution + 0.5);
                int bbh = (int)((float)m_height * 72.0 / m_resolution + 0.5);

                // take width and height as parameters for the bounding box
                m_widget.edit_bb->setText(QString("0 0 ") + QString::number(bbw)
                                          + ' '
                                          + QString::number(bbh));

                // show information
                setInfo();

            }
        }
    }
}

void IncludeGraphics::onAccepted()
{
    writeConfig();
}

void IncludeGraphics::onWrapFigureSelected(bool state) {
    if (m_widget.cb_figure->isChecked() && state) {
        m_widget.cb_figure->setChecked(false);
    }
    // Adds warning to log if wrapfig isn't in the preamble
    QStringList packagelist = m_ki->allPackages();
    if (!packagelist.contains("wrapfig")) {
        m_ki->errorHandler()->printMessage(KileTool::Error, i18n("You must include the wrapfig package to use the text wrapping options"), i18n("Missing Package"));
    }
}

void IncludeGraphics::onFigureSelected(bool state) {
    if (m_widget.cb_wrapfigure->isChecked() && state) {
        m_widget.cb_wrapfigure->setChecked(false);
    }
}
}
