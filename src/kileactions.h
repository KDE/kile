/*************************************************************************************
    begin                :  2003-07-01 17:33:00 CEST 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2008-2019 by Michel Ludwig (michel.ludwig@kdemail.net)
                           (C) 2009 Thomas Braun (thomas.braun@virtuell-zuhause.de)

 *************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KILEACTIONS_H
#define KILEACTIONS_H

#include <QAction>
#include <KActionCollection>
#include <KActionMenu>
#include <KSelectAction>
#include <QDialog>
#include <QLineEdit>

class QCheckBox;

class KileInfo;

namespace KileAction
{

enum { KeepHistory=1, ShowAlternative=2, ShowBrowseButton=4, FromLabelList=8, FromBibItemList=16, ShowLabel=32, AddProjectFile=64};
/*
	TagData
*/

class TagData
{
public:
    explicit TagData(const QString &t, const QString &tB = QString(), const QString &tE = QString(), int x = 0, int y = 0, const QString &desc = QString())
        : text(t), tagBegin(tB), tagEnd(tE), dx(x), dy(y), description(desc) {}

    TagData() : text(QString()), tagBegin(QString()), tagEnd(QString()), dx(0), dy(0), description(QString()) {}

    QString		text;
    QString		tagBegin, tagEnd;
    int			dx,dy;
    QString		description;
};

class Tag : public QAction
{
    Q_OBJECT

public:
    //constructors
    Tag(const QString &text, const QString& iconText, const QKeySequence &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const QString& name,
        const QString &tagBegin, const QString &tagEnd = QString(), int dx = 0, int dy = 0, const QString &description = QString());

    Tag(const QString &text, const QString& iconText, const QString& pix, const QKeySequence &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const QString& name,
        const QString &tagBegin, const QString &tagEnd = QString(), int dx = 0, int dy = 0, const QString &description = QString());

    Tag(const QString &text, const QString& iconText, const QKeySequence &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const QString& name,
        const TagData& data);

    Tag(const QString &text, const QString& iconText, const QString& pix, const QKeySequence &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const QString& name,
        const TagData& data);

    ~Tag();

private:
    void init(const QObject *receiver = Q_NULLPTR, const char *slot = Q_NULLPTR);

Q_SIGNALS:
    //sends along tagdata so that receiver knows what to insert
    void triggered(const KileAction::TagData&);

private Q_SLOTS:
    //emits the triggered(TagData) signal
    virtual void emitData();

protected:
    TagData m_data;
};

/*
	InputTag: adds a history list and options for a input dialog to TagData
*/
class InputTag : public Tag
{
    Q_OBJECT

public:
    //constructors
    InputTag(KileInfo* ki, const QString &text, const QString &iconText, const QKeySequence &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const QString& name, QWidget *wparent, uint options,
             const QString &tagBegin, const QString &tagEnd = QString(), int dx = 0, int dy = 0, const QString &description = QString(), const QString &hint = QString(), const QString &alter = QString());

    InputTag(KileInfo* ki, const QString &text, const QString &iconText, const QString& pix, const QKeySequence &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const QString& name, QWidget *wparent, uint options,
             const QString &tagBegin, const QString &tagEnd = QString(), int dx = 0, int dy = 0, const QString &description = QString(), const QString &hint = QString(), const QString &alter = QString());

    InputTag(KileInfo* ki, const QString &text, const QString &iconText, const QKeySequence &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const QString& name, QWidget *wparent, uint options,
             const TagData& data, const QString &hint = QString(), const QString &alter = QString());

    InputTag(KileInfo* ki, const QString &text, const QString &iconText, const QString& pix, const QKeySequence &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const QString& name, QWidget *wparent, uint options,
             const TagData& data, const QString &hint = QString(), const QString &alter = QString());

    ~InputTag();

    bool hasHistory() {
        return (m_options & KeepHistory);
    }
    bool hasAlternative() {
        return (m_options & ShowAlternative);
    }
    bool hasBrowseButton() {
        return (m_options & ShowBrowseButton);
    }

    void addToHistory(const QString& str);

private:
    void init();

private Q_SLOTS:
    //emits the triggered(TagData) signal
    virtual void emitData() override;

private:
    KileInfo	*m_ki;
    QStringList			m_history;
    QWidget				*m_parent;
    uint				m_options;
    QString				m_hint;
    QString				m_alter;
};


/*
	InputDialog
*/
class InputDialog : public QDialog
{
    Q_OBJECT

public:
    InputDialog(const QString &caption, uint options, const QStringList& history, const QString &hint, const QString &alter, KileInfo *ki, QWidget *parent=0, const char *name=0);
    ~InputDialog();

    bool useAlternative() {
        return m_useAlternative;
    }
    bool useLabel() {
        return m_useLabel;
    }
    bool useAddProjectFile() {
        return m_useAddProjectFile;
    }

public Q_SLOTS:
    void slotBrowse();
    void slotAltClicked();

    void setTag(const QString&);

Q_SIGNALS:
    void setInput(const QString&);

public:
    QString tag() {
        return m_tag;
    }
    QString label();
    bool usedSelection() {
        return m_usedSelection;
    }

    QLineEdit *m_edLabel;

private:
    QString	m_tag;
    QString	m_labelprefix;
    bool		m_useAlternative,m_useLabel,m_usedSelection,m_useAddProjectFile;
    KileInfo	*m_ki;
};

class Select : public KSelectAction
{
    Q_OBJECT

public:
    //constructors
    Select(const QString &text, const QKeySequence &cut, KActionCollection *parent, const char *name);

public Q_SLOTS:
    void setItems(const QList<QAction *> &);

};

class VariantSelection : public QAction
{
    Q_OBJECT

public:
    VariantSelection(const QString &text, const QVariant& value, QObject *parent = Q_NULLPTR);

Q_SIGNALS:
    void triggered(const QVariant& value);
    void triggered(const QUrl &url);
    void triggered(const QString& string);

private Q_SLOTS:
    void slotTriggered();

private:
    QVariant m_variant;
};

}

class ToolbarSelectAction : public QWidgetAction
{
    Q_OBJECT

public:
    ToolbarSelectAction(const QString& text, QObject* parent, bool changeMainActionOnTriggering = true);

    void addAction(QAction *action);
    void addSeparator();
    int actionIndex(QAction *action);
    QAction* action(int i);
    QAction* currentAction();
    bool containsAction(QAction *action);
    int currentItem() const;
    void setCurrentItem(int i);
    void setCurrentAction(QAction *action);
    void removeAllActions();

    void saveCurrentAction();
    void restoreCurrentAction();

protected Q_SLOTS:
    void slotTriggered(QAction*);
    void slotMainActionTriggered();
    void slotMainButtonPressed();

Q_SIGNALS:
    void mainButtonWithNoActionPressed();

protected:
    QMenu* menu();
    virtual QWidget* createWidget(QWidget *parent) override;

private:
    QList<QAction*> m_actionList;
    int m_currentItem;
    QString m_mainText;
    QAction *m_savedCurrentAction;
};

#endif
