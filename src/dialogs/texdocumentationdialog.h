/***************************************************************************
                         texdocdialog.h
                         --------------
    date                 : Feb 15 2007
    version              : 0.14
    copyright            : (C) 2005-2007 by Holger Danielsson
    email                : holger.danielsson@versanet.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TEXDOCUMENTATIONDIALOG_H
#define TEXDOCUMENTATIONDIALOG_H

#include <QDialog>

#include <QMap>
#include <QProcess>

class QTreeWidget;
class QTreeWidgetItem;

class KProcess;
class QDialogButtonBox;
class QLineEdit;
class QPushButton;
class QTemporaryFile;

namespace KileDialog
{

class TexDocDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TexDocDialog(QWidget *parent = 0);
    ~TexDocDialog();

private:
    QTreeWidget *m_texdocs;
    QLineEdit *m_leKeywords;
    QPushButton *m_pbSearch;
    QDialogButtonBox *m_buttonBox;

    QString m_texmfPath, m_texmfdocPath, m_texdoctkPath;

    QStringList m_tocList, m_tocSearchList;
    QMap<QString, QString> m_dictDocuments;
    QMap<QString, QString> m_dictStyleCodes;

    void readToc();
    void showToc(const QString &caption, const QStringList &doclist, bool toc = true);

    QString m_filename;
    QString m_output;

    QTemporaryFile *m_tempfile;
    KProcess *m_proc;

    void callSearch();
    void executeScript(const QString &command);
    void showFile(const QString &filename);

    QString searchFile(const QString &docfilename, const QString &listofpathes,
                       const QString &subdir = QString());
    void decompressFile(const QString &docfile, const QString &command);
    void showStyleFile(const QString &filename, const QString &stylecode);

    QString getMimeType(const QString &filename);
    QString getIconName(const QString &filename);

protected:
    virtual bool eventFilter(QObject *o, QEvent *e) override;

Q_SIGNALS:
    void processFinished();

private Q_SLOTS:
    void slotListViewDoubleClicked(QTreeWidgetItem *item);
    void slotTextChanged(const QString &text);
    void slotSearchClicked();
    void slotResetSearch();

    void slotProcessOutput();
    void slotProcessExited(int, QProcess::ExitStatus);

    void slotInitToc();
    void slotShowFile();
};

}

#endif
