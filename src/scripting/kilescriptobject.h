/******************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 ******************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KILE_SCRIPT_OBJECT_H
#define KILE_SCRIPT_OBJECT_H

#include <QObject>
#include <QAction>
#include <QMap>
#include <QStringList>
#include <QUrl>

#include <KParts/MainWindow>

class KileInfo;
class KileMainWindow;


namespace KileScript {

////////////////////////////////// KileAlert object //////////////////////////////////////

class KileAlert : public QObject
{
    Q_OBJECT

public:
    KileAlert(QObject *parent, KParts::MainWindow *mainWindow);
    virtual ~KileAlert() {}

    Q_INVOKABLE void information(const QString &text, const QString &caption = QString());
    Q_INVOKABLE void sorry(const QString &text, const QString &caption = QString());
    Q_INVOKABLE void error(const QString &text, const QString &caption = QString());
    Q_INVOKABLE QString question(const QString &text, const QString &caption = QString());
    Q_INVOKABLE QString warning(const QString &text, const QString &caption = QString());

private:
    KParts::MainWindow *m_mainWindow;
};

////////////////////////////////// KileInput object //////////////////////////////////////

class KileInput : public QObject
{
    Q_OBJECT

public:
    explicit KileInput(QObject *parent);
    virtual ~KileInput() {}

    // get input values
    Q_INVOKABLE QString getListboxItem(const QString &caption, const QString &label, const QStringList &list);
    Q_INVOKABLE QString getText(const QString &caption, const QString &label);
    Q_INVOKABLE QString getLatexCommand(const QString &caption, const QString &label);
    Q_INVOKABLE int getInteger(const QString &caption, const QString &label, int min = INT_MIN, int max = INT_MAX);
    Q_INVOKABLE int getPosInteger(const QString &caption, const QString &label, int min = 1, int max = INT_MAX);

private:
    KileMainWindow *m_mainWindow;
    QString getItem(const QString& caption, const QString& label, const QStringList &itemlist);
    QStringList checkCaptionAndLabel(const QString& caption, const QString& label);
};

////////////////////////////////// KileWizard object //////////////////////////////////////

class KileWizard : public QObject
{
    Q_OBJECT

public:
    KileWizard(QObject *parent, KileInfo *kileInfo, const QMap<QString,QAction *> *scriptActions);
    virtual ~KileWizard() {}

    // wizards
    Q_INVOKABLE void tabular();
    Q_INVOKABLE void array();
    Q_INVOKABLE void tabbing();
    Q_INVOKABLE void floatEnvironment();
    Q_INVOKABLE void mathEnvironment();
    Q_INVOKABLE void postscript();
    Q_INVOKABLE void pdf();

private:
    KileInfo *m_kileInfo;
    const QMap<QString,QAction *> *m_scriptActions;

    bool triggerAction(const QString &name);

};

////////////////////////////////// KileJavaScript  object //////////////////////////////////////

class KileJavaScript : public QObject
{
    Q_OBJECT

public:
    explicit KileJavaScript(QObject *parent);
    virtual ~KileJavaScript() {}

    void setScriptname(const QString &name) {
        m_scriptname = name;
    }

    Q_INVOKABLE QString name() const {
        return m_scriptname;
    }
    Q_INVOKABLE QString caption();

private:
    QString m_scriptname;

};

////////////////////////////////// KileFile object //////////////////////////////////////

class KileFile : public QObject
{
    Q_OBJECT

public:

    enum AccessResult {
        ACCESS_OK = 0,
        ACCESS_FAILED,
        ACCESS_DENIED
    };

    KileFile(QObject *parent, KileInfo *kileInfo);
    virtual ~KileFile() {}

    // Read the contents of a file. If no filename is given, getOpenFileName() is called first
    Q_INVOKABLE QMap<QString, QVariant> read(const QString& filename) const;
    Q_INVOKABLE QMap<QString, QVariant> read() const;

    // Write text into a file. If no filename is given, getSaveFileName() is called first
    Q_INVOKABLE QMap<QString, QVariant> write(const QString& filename, const QString& text) const;
    Q_INVOKABLE QMap<QString, QVariant> write(const QString& text) const;

    // Open/Save dialogs from KFileDialog
    Q_INVOKABLE QString getOpenFileName(const QUrl &url = QUrl(), const QString& filter = QString());
    Q_INVOKABLE QString getSaveFileName(const QUrl &url = QUrl(), const QString& filter = QString());

private:
    KileInfo *m_kileInfo;

    QMap<QString, QVariant> actionCancelled() const;
};

////////////////////////////////// KileScript object //////////////////////////////////////

class KileScriptObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* alert READ getAlert);
    Q_PROPERTY(QObject* input READ getInput);
    Q_PROPERTY(QObject* wizard READ getWizard);
    Q_PROPERTY(QObject* script READ getScript);
    Q_PROPERTY(QObject* file READ getFile);

public:
    KileScriptObject(QObject *parent, KileInfo *kileInfo, const QMap<QString,QAction *> *scriptActions);
    virtual ~KileScriptObject() {}

    QObject* getAlert()  {
        return m_kileAlert;
    }
    QObject* getInput()  {
        return m_kileInput;
    }
    QObject* getWizard() {
        return m_kileWizard;
    }
    QObject* getScript() {
        return m_kileScript;
    }
    QObject* getFile()   {
        return m_kileFile;
    }

    void setScriptname(const QString &name);

private:
    KileInfo* m_kileInfo;

    KileAlert  *m_kileAlert;
    KileInput  *m_kileInput;
    KileWizard *m_kileWizard;
    KileJavaScript *m_kileScript;
    KileFile *m_kileFile;
};

}

#endif

