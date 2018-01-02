/************************************************************************************************
  Copyright (C) 2004-2007 by Holger Danielsson (holger.danielsson@versanet.de)
                2008-2014 by Michel Ludwig (michel.ludwig@kdemail.net)
 ************************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CODECOMPLETION_H
#define CODECOMPLETION_H

#include <QObject>
#include <QList>

#include <KTextEditor/CodeCompletionInterface>
#include <KTextEditor/CodeCompletionModel>
#include <KTextEditor/CodeCompletionModelControllerInterface>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <kconfig.h>

#include "latexcmd.h"
#include "widgets/abbreviationview.h"

//default bullet char (a cross)
static const QChar s_bullet_char = QChar(0xd7);
static const QString s_bullet = QString(&s_bullet_char, 1);

class QTimer;

class KileInfo;

namespace KileDocument {
class EditorExtension;
}

namespace KileCodeCompletion
{
class Manager;

class LaTeXCompletionModel : public KTextEditor::CodeCompletionModel,
    public KTextEditor::CodeCompletionModelControllerInterface {
    Q_OBJECT
    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)

public:
    LaTeXCompletionModel(QObject *parent, KileCodeCompletion::Manager *manager,
                         KileDocument::EditorExtension *editorExtension);
    virtual ~LaTeXCompletionModel();
    virtual QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    virtual bool shouldStartCompletion(KTextEditor::View *view, const QString &insertedText,
                                       bool userInsertion, const KTextEditor::Cursor &position) Q_DECL_OVERRIDE;
    virtual bool shouldAbortCompletion(KTextEditor::View *view, const KTextEditor::Range &range,
                                       const QString &currentCompletion) Q_DECL_OVERRIDE;
    virtual void completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range,
                                   InvocationType invocationType) Q_DECL_OVERRIDE;
    virtual KTextEditor::Range updateCompletionRange(KTextEditor::View *view,
            const KTextEditor::Range& range) Q_DECL_OVERRIDE;
    virtual KTextEditor::Range completionRange(KTextEditor::View *view,
            const KTextEditor::Cursor &position) Q_DECL_OVERRIDE;
    virtual QString filterString(KTextEditor::View *view,
                                 const KTextEditor::Range &range,
                                 const KTextEditor::Cursor &position) Q_DECL_OVERRIDE;

    virtual void executeCompletionItem(KTextEditor::View *view, const KTextEditor::Range& word,
                                       const QModelIndex &index) const Q_DECL_OVERRIDE;
    QString filterLatexCommand(const QString &text, int &cursorYPos, int &cursorXPos);


protected:
    KileCodeCompletion::Manager *m_codeCompletionManager;
    KileDocument::EditorExtension *m_editorExtension;
    QStringList m_completionList;
    KTextEditor::View *m_currentView;

    void buildModel(KTextEditor::View *view, const KTextEditor::Range &r);
    void filterModel(const QString& text);

    QString stripParameters(const QString &text) const;
    QString buildRegularCompletedText(const QString &text, int &cursorYPos, int &cursorXPos,
                                      bool checkGroup) const;
    QString buildEnvironmentCompletedText(const QString &text, const QString &prefix,
                                          int &ypos, int &xpos) const;
    QString buildWhiteSpaceString(const QString &s) const;
    KTextEditor::Cursor determineLaTeXCommandStart(KTextEditor::Document *doc,
            const KTextEditor::Cursor& position) const;
    bool isWithinLaTeXCommand(KTextEditor::Document *doc, const KTextEditor::Cursor& commandStart,
                              const KTextEditor::Cursor& cursorPosition) const;
};

class AbbreviationCompletionModel : public KTextEditor::CodeCompletionModel, public KTextEditor::CodeCompletionModelControllerInterface {
    Q_OBJECT
    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)

public:
    AbbreviationCompletionModel(QObject *parent, KileAbbreviation::Manager *manager);
    virtual ~AbbreviationCompletionModel();

    virtual QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    virtual bool shouldStartCompletion(KTextEditor::View *view, const QString &insertedText,
                                       bool userInsertion, const KTextEditor::Cursor &position) Q_DECL_OVERRIDE;
    virtual bool shouldAbortCompletion(KTextEditor::View *view, const KTextEditor::Range &range,
                                       const QString &currentCompletion) Q_DECL_OVERRIDE;
    virtual void completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range,
                                   InvocationType invocationType) Q_DECL_OVERRIDE;
    virtual KTextEditor::Range updateCompletionRange(KTextEditor::View *view,
            const KTextEditor::Range& range) Q_DECL_OVERRIDE;
    virtual KTextEditor::Range completionRange(KTextEditor::View *view,
            const KTextEditor::Cursor &position) Q_DECL_OVERRIDE;
    virtual QString filterString(KTextEditor::View *view,
                                 const KTextEditor::Range &range,
                                 const KTextEditor::Cursor &position) Q_DECL_OVERRIDE;

    virtual void executeCompletionItem(KTextEditor::View *view, const KTextEditor::Range& word,
                                       const QModelIndex &index) const Q_DECL_OVERRIDE;

protected:
    KileAbbreviation::Manager *m_abbreviationManager;
    QStringList m_completionList;
    KTextEditor::View *m_currentView;

    /**
     * 'singleMatchMode' should be set to true for manual invocations of this completion model;
     * it triggers an immediate substitution of a matching abbreviation even if there is a longer
     * abbreviation match, e.g. for two abbreviations 'a' -> 'x1' and 'ab' -> 'x2', 'singleMatchMode'
     * will immediately replace 'a' with 'x1' if 'a' has been entered by the user.
     **/
    void buildModel(KTextEditor::View *view, const KTextEditor::Range &r, bool singleMatchMode = false);
};

class Manager : public QObject {
    Q_OBJECT
    friend class LaTeXCompletionModel;

public:
    Manager(KileInfo *info, QObject *parent);
    virtual ~Manager();

    QStringList getLaTeXCommands() const;
    QStringList getLocallyDefinedLaTeXCommands(KTextEditor::View *view) const;

    void readConfig(KConfig *config);

    QStringList readCWLFile(const QString &filename, bool fullPathGiven = false);
    QStringList readCWLFiles(const QStringList &files, const QString &dir);
    QString validCwlFile(const QString &filename);

    /**
     * A map from file names to paths is returned.
     */
    static QMap<QString, QString> getAllCwlFiles(const QString &localCwlPath, const QString &globalCwlPath);
    /**
     * Returns a pair (local CWL base dir, global CWL base dir).
     */
    static QPair<QString, QString> getCwlBaseDirs();

public Q_SLOTS:
    void startLaTeXCompletion(KTextEditor::View *view = Q_NULLPTR);
    void startLaTeXEnvironment(KTextEditor::View *view = Q_NULLPTR);
    void startAbbreviationCompletion(KTextEditor::View *view = Q_NULLPTR);

    void textInserted(KTextEditor::View* view, const KTextEditor::Cursor& position, const QString & text);

protected:
    KileInfo* m_ki;
    QStringList m_texWordList, m_dictWordList, m_abbrevWordList;
    bool m_firstConfig;
    QRegExp m_referencesRegExp;
    QRegExp m_referencesExtRegExp;
    QRegExp m_citeRegExp;
    QRegExp m_citeExtRegExp;

    void addUserDefinedLaTeXCommands(QStringList &wordlist);
    void buildReferenceCitationRegularExpressions();
    QString getCommandsString(KileDocument::CmdAttribute attrtype);
};

}
#endif
