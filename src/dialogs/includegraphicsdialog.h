/***************************************************************************
    date                 : Nov 02 2005
    version              : 0.23
    copyright            : 2004-2005  Holger Danielsson <holger.danielsson@t-online.de>
                           2004       Jeroen Wijnhout
                           2015       Andreas Cord-Landwehr <cordlandwehr@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef INCLUDEGRAPHICSDIALOG_H
#define INCLUDEGRAPHICSDIALOG_H

#include <QDialog>
#include <QProcess>

#include "ui_includegraphicsdialog_base.h"

class QDialogButtonBox;
class KProcess;
class KileInfo;

/**
  *@author dani
  */
namespace KileDialog
{

class IncludeGraphics : public QDialog
{
    Q_OBJECT

public:
    IncludeGraphics(QWidget *parent, const QString &startdir, KileInfo *ki);
    virtual ~IncludeGraphics();

    QString getTemplate();
    QString getFilename();

private Q_SLOTS:
    void onChooseFilter();
    void onUrlSelected(const QUrl &url);
    void onTextChanged(const QString& string);
    void onProcessOutput();
    void onProcessExited(int exitCode, QProcess::ExitStatus exitStatus);

    void onWrapFigureSelected(bool state);
    void onFigureSelected(bool state);
    void onAccepted();

private:
    void readConfig();
    void writeConfig();
    QString getOptions();
    QString getInfo();
    bool getPictureSize(int &wpx, int &hpx, QString &dpi, QString &wcm, QString &hcm);
    void setInfo();

    QDialogButtonBox *m_buttonBox;
    Ui::IncludeGraphicsWidget m_widget;

    QString m_startdir;
    QString m_output;

    // current picture
    int m_width, m_height;
    float m_resolution;

    // default
    bool m_imagemagick;
    bool m_boundingbox;
    float m_defaultresolution;

    void execute(const QString &command);

    KileInfo *m_ki;
    KProcess* m_proc;
};

}

#endif
