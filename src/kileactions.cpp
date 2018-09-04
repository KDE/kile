/**************************************************************************************
    begin                :  2003-07-01 17:33:00 CEST 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2008-2016 by Michel Ludwig (michel.ludwig@kdemail.net)
 **************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// 2005-07-26 dani
//  - cleanup dialog
//  - added new action 'ShowLabel'

// 2007-03-12 dani
//  - use KileDocument::Extensions

#include "kileactions.h"

#include <QCheckBox>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QToolBar>

#include <QMenu>

#include <KComboBox>
#include <KCompletion>
#include <KLocalizedString>
#include <KIconLoader>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QFileDialog>

#include "kiledebug.h"
#include "kileextensions.h"
#include "kileinfo.h"
#include "kiledocmanager.h"

namespace KileAction
{

////////////////
//    Tag     //
////////////////
Tag::Tag(const QString &text, const QString& iconText, const QKeySequence &shortcut, const QObject *receiver, const char *slot, KActionCollection *parent,
         const QString& name, const QString &tagBegin, const QString &tagEnd,
         int dx, int dy, const QString &description)
    : QAction(text, parent),
      m_data(text,tagBegin, tagEnd, dx, dy, description)
{
    parent->addAction(name, this);
    setIconText(iconText);
    if(!shortcut.isEmpty()) {
        parent->setDefaultShortcut(this, shortcut);
    }
    init(receiver, slot);
}

Tag::Tag(const QString &text, const QString& iconText, const QString& pix, const QKeySequence &shortcut, const QObject *receiver, const char *slot, KActionCollection *parent,
         const QString& name, const QString &tagBegin, const QString &tagEnd,
         int dx, int dy, const QString &description)
    : QAction(QIcon::fromTheme(pix), text, parent),
      m_data(text,tagBegin, tagEnd, dx, dy, description)
{
    parent->addAction(name, this);
    setIconText(iconText);
    if(!shortcut.isEmpty()) {
        parent->setDefaultShortcut(this, shortcut);
    }
    init(receiver,slot);
}

Tag::Tag(const QString &text, const QString& iconText, const QKeySequence &shortcut, const QObject *receiver, const char *slot, KActionCollection *parent,
         const QString& name, const TagData& data)
    : QAction(text, parent),
      m_data(data)
{
    parent->addAction(name, this);
    setIconText(iconText);
    if(!shortcut.isEmpty()) {
        parent->setDefaultShortcut(this, shortcut);
    }
    init(receiver,slot);
}

Tag::Tag(const QString &text, const QString& iconText, const QString& pix, const QKeySequence &shortcut, const QObject *receiver, const char *slot, KActionCollection *parent,
         const QString& name, const TagData& data)
    : QAction(QIcon::fromTheme(pix), text, parent),
      m_data(data)
{
    parent->addAction(name, this);
    if(!shortcut.isEmpty()) {
        parent->setDefaultShortcut(this, shortcut);
    }
    setIconText(iconText);
    init(receiver,slot);
}

Tag::~Tag()
{
}

void Tag::init(const QObject *receiver, const char *slot)
{
    connect(this, SIGNAL(triggered()), SLOT(emitData()));
    connect(this, SIGNAL(triggered(KileAction::TagData)), receiver, slot);
}

void Tag::emitData()
{
    emit(triggered(m_data));
}

////////////////
//    InputTag     //
////////////////
InputTag::InputTag(KileInfo* ki, const QString &text, const QString &iconText, const QKeySequence &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const QString& name, QWidget *wparent, uint options,
                   const QString &tagBegin, const QString &tagEnd,
                   int dx, int dy, const QString &description, const QString &hint, const QString &alter)
    : Tag(text, iconText, cut, receiver, slot, parent, name, tagBegin, tagEnd, dx, dy, description), m_ki(ki),
      m_parent(wparent), m_options(options), m_hint(hint), m_alter(alter)
{
    init();
}

InputTag::InputTag(KileInfo* ki, const QString &text, const QString &iconText, const QString& pix, const QKeySequence &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const QString& name, QWidget *wparent, uint options,
                   const QString &tagBegin, const QString &tagEnd,
                   int dx, int dy, const QString &description, const QString &hint, const QString &alter)
    : Tag(text, iconText, pix, cut, receiver, slot, parent, name, tagBegin, tagEnd, dx, dy, description), m_ki(ki),
      m_parent(wparent), m_options(options), m_hint(hint), m_alter(alter)
{
    init();
}

InputTag::InputTag(KileInfo* ki, const QString &text, const QString &iconText, const QKeySequence &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const QString& name, QWidget *wparent, uint options,
                   const TagData& data, const QString &hint, const QString &alter)
    : Tag(text, iconText, cut, receiver, slot, parent, name,data),  m_ki(ki),
      m_parent(wparent), m_options(options), m_hint(hint), m_alter(alter)
{
    init();
}

InputTag::InputTag(KileInfo* ki, const QString &text, const QString &iconText, const QString& pix, const QKeySequence &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const QString& name, QWidget *wparent, uint options,
                   const TagData& data, const QString &hint, const QString &alter)
    : Tag(text, iconText, pix, cut,receiver, slot, parent, name,data), m_ki(ki),
      m_parent(wparent), m_options(options), m_hint(hint), m_alter(alter)
{
    init();
}

InputTag::~InputTag()
{
}

void InputTag::init()
{
    m_history.clear();
}

void InputTag::addToHistory(const QString& str)
{
    if(!m_history.contains(str)) {
        m_history.prepend(str);
    }
}

void InputTag::emitData()
{
    KILE_DEBUG_MAIN << "InputTag::emitData() " << m_ki->getName();

    InputDialog *dlg = new InputDialog(m_data.text, m_options, m_history, m_hint, m_alter, m_ki, m_parent, "input_dialog");
    if (dlg->exec()) {
        if((!dlg->tag().isEmpty()) && hasHistory()) {
            addToHistory(dlg->tag());
        }

        TagData td(m_data);

        td.tagBegin.replace("%R",dlg->tag());
        td.tagEnd.replace("%R",dlg->tag());

        QString alt = dlg->useAlternative() ? "*" : "";
        td.tagBegin.replace("%A", alt);
        td.tagEnd.replace("%A", alt);

        if(dlg->useLabel()) {
            td.tagEnd += dlg->label();
            td.dy++;
        }

        if(dlg->usedSelection()) {
            m_ki->clearSelection();
        }

        // if a filename was given for a \input- or \include-command,
        // the cursor is moved out of the braces
        if ( (m_options & (KileAction::ShowBrowseButton | KileAction::FromLabelList | KileAction::FromBibItemList)) && !dlg->tag().isEmpty() ) {
            td.dx += dlg->tag().length() + 1;
        }

        // insert tag
        emit(triggered(td));
        // refresh document structure and project tree when a file was inserted
        if(dlg->useAddProjectFile()) {
            m_ki->docManager()->projectAddFile(QFileInfo(m_ki->getCompileName()).absolutePath() + '/' + dlg->tag());
        }
    }
    delete dlg;
}

/*
	InputDialog
*/
InputDialog::InputDialog(const QString &caption, uint options, const QStringList& history, const QString& hint, const QString& alter, KileInfo *ki, QWidget *parent, const char *name)
    : QDialog (parent), m_ki(ki)
{
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    setObjectName(name);

    QString newcaption = caption;
    setWindowTitle(newcaption.remove('&'));

    m_labelprefix = (newcaption == "chapter") ? "chap:" : "sec:";
    m_usedSelection = false;

    QGridLayout *gbox = new QGridLayout(this);
    QWidget *page = new QWidget(this);
    page->setLayout(gbox);
    mainLayout->addWidget(page);

    QLabel *lb = new QLabel(hint, this);
    gbox->addWidget(lb, 0, 0, 1, 3);

    m_tag.clear();
    QWidget *focus;
    if((options & KileAction::KeepHistory) || (options & KileAction::FromLabelList) || (options & KileAction::FromBibItemList)) {
        KComboBox *input = new KComboBox(true, this);
        input->setObjectName("input_dialog_input");
        input->setCompletionMode(KCompletion::CompletionAuto);
        input->setMinimumWidth(300);
        focus = input;

        connect(input, SIGNAL(textChanged(QString)), this, SLOT(setTag(QString)));
        connect(this,  SIGNAL(setInput(QString)), input, SLOT(setEditText(QString)));
        if(options & KileAction::ShowBrowseButton) {
            gbox->addWidget(input, 1, 0);
        }
        else {
            gbox->addWidget(input, 1, 0, 1, 3);
        }

        QStringList list;

        if(options & KileAction::FromLabelList) {
            list = ki->allLabels();
            if(list.size() > 0) {
                input->addItems(list);
                m_tag = list.first();
            }
        }
        else if(options & KileAction::FromBibItemList) {
            list = ki->allBibItems();
            if(list.size() > 0) {
                input->addItems(list);
                m_tag = list.first();
            }
        }
        else {
            if(history.size() > 0) {
                input->addItems(history);
                m_tag = history.first();
            }
        }
    }
    else {
        QLineEdit *input = new QLineEdit(this);
        gbox->addWidget(input);
        input->setMinimumWidth(300);
        focus = input;

        connect(input, SIGNAL(textChanged(QString)), this, SLOT(setTag(QString)));
        connect(this,  SIGNAL(setInput(QString)), input, SLOT(setText(QString)));
        if(options & KileAction::ShowBrowseButton) {
            gbox->addWidget(input, 1, 0);
        }
        else {
            gbox->addWidget(input, 1, 0, 1, 3);
        }

        input->setText(ki->getSelection());
        m_usedSelection=true;
    }

    if(focus) {
        lb->setBuddy(focus);
    }

    if(options & KileAction::ShowBrowseButton) {
        QPushButton *pbutton = new QPushButton(QString(), this);
        mainLayout->addWidget(pbutton);
        pbutton->setIcon(QIcon::fromTheme("document-open"));
        gbox->addWidget(pbutton, 1, 2);
        gbox->setColumnMinimumWidth(1, 8);
        gbox->setColumnMinimumWidth(2, pbutton->sizeHint().width() + 5);
        connect(pbutton, SIGNAL(clicked()), this, SLOT(slotBrowse()));
    }

    if(options & KileAction::ShowAlternative) {
        QCheckBox *m_checkbox = new QCheckBox(alter, this);
        mainLayout->addWidget(m_checkbox);
        m_checkbox->setObjectName("input_dialog_checkbox");
        connect(m_checkbox, SIGNAL(clicked()), this, SLOT(slotAltClicked()));
        m_useAlternative=false;
        gbox->addWidget(m_checkbox, 2, 0, 1, 3);
    }

    m_edLabel = Q_NULLPTR;
    m_useLabel = (options & KileAction::ShowLabel);
    if(m_useLabel) {
        // Label
        QLabel *label = new QLabel(i18n("&Label:"),this);
        mainLayout->addWidget(label);
        m_edLabel = new QLineEdit(this);
        mainLayout->addWidget(m_edLabel);
        m_edLabel->setMinimumWidth(300);
        m_edLabel->setText(m_labelprefix);
        label->setBuddy(m_edLabel);
        gbox->addWidget(label, 3, 0, 1, 3);
        gbox->addWidget(m_edLabel, 4, 0, 1, 3);
    }

    m_useAddProjectFile = (options & KileAction::AddProjectFile);

    gbox->setRowStretch(5, 1);
    gbox->setColumnStretch(0, 1);

    // add buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    focus->setFocus();
}


InputDialog::~InputDialog()
{
}

void InputDialog::slotBrowse()
{
    QString fn;
    QFileInfo fi(m_ki->getCompileName());

    // Called from InputDialog after a \input- or \include command:
    // so we are only looking for a LaTeX source document
    QString filter = m_ki->extensions()->fileFilterQtStyle(true, {KileDocument::Extensions::TEX});

    fn = QFileDialog::getOpenFileName(this, i18n("Select File"), fi.absoluteFilePath(), filter);
    if(!fn.isEmpty()) {
        QString path = QDir(fi.path()).relativeFilePath(fn);

        // if the file has no extension, we add the default TeX extension
        if(QFileInfo(path).completeSuffix().isEmpty()) {
            path += m_ki->extensions()->latexDocumentDefault();
        }

        setTag(path);
        emit(setInput(path));
    }
}

void InputDialog::slotAltClicked()
{
    m_useAlternative = !m_useAlternative;
}

void InputDialog::setTag(const QString &tag)
{
    m_tag = tag;
}

QString InputDialog::label()
{
    if(m_edLabel) {
        QString label = m_edLabel->text().trimmed();
        if(!label.isEmpty() && label != m_labelprefix) {
            return "\\label{" + label + "}\n";
        }
    }

    return QString();
}

/////////////////
//  SelectTag  //
/////////////////

Select::Select(const QString &text, const QKeySequence &shortcut, KActionCollection *parent, const char *name)
    : KSelectAction(text, parent)
{
    parent->addAction(name, this);
    parent->setDefaultShortcut(this, shortcut);
}

void Select::setItems(const QList<QAction *>& list)
{
    removeAllActions();

    for(QList<QAction *>::const_iterator i = list.begin(); i != list.end(); ++i) {
        addAction(*i);
    }
}

/////////////////////////
//  VariantSelection   //
/////////////////////////

VariantSelection::VariantSelection(const QString &text, const QVariant& value, QObject *parent)
    : QAction(text, parent), m_variant(value)
{
    connect(this, SIGNAL(triggered(bool)), this, SLOT(slotTriggered()));
}

void VariantSelection::slotTriggered()
{
    emit(triggered(m_variant));

    if(m_variant.canConvert<QUrl>()) {
        emit(triggered(m_variant.value<QUrl>()));
    }

    if(m_variant.canConvert<QString>()) {
        emit(triggered(m_variant.value<QString>()));
    }
}

}

// ToolbarSelectAction
// based on 'KActionMenu', therefore our thanks go to
// Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
//           (C) 1999 Simon Hausmann <hausmann@kde.org>
//           (C) 2000 Nicolas Hadacek <haadcek@kde.org>
//           (C) 2000 Kurt Granroth <granroth@kde.org>
//           (C) 2000 Michael Koch <koch@kde.org>
//           (C) 2001 Holger Freyther <freyther@kde.org>
//           (C) 2002 Ellis Whitehead <ellis@kde.org>
//           (C) 2002 Joseph Wenninger <jowenn@kde.org>
//           (C) 2003 Andras Mantia <amantia@kde.org>
//           (C) 2005-2006 Hamish Rodda <rodda@kde.org>

ToolbarSelectAction::ToolbarSelectAction(const QString& text, QObject* parent,
        bool changeMainActionOnTriggering /*= true */)
    : QWidgetAction(parent), m_currentItem(-1), m_mainText(text), m_savedCurrentAction(Q_NULLPTR)
{
    setText(text);
    if(changeMainActionOnTriggering) {
        connect(menu(), SIGNAL(triggered(QAction*)), this, SLOT(slotTriggered(QAction*)));
    }
}

int ToolbarSelectAction::actionIndex(QAction *action)
{
    int counter = -1;
    QList<QAction*> actionList = menu()->actions();
    for(QList<QAction*>::iterator i = actionList.begin(); i != actionList.end(); ++i) {
        if(*i == action) {
            return counter + 1;
        }
        ++counter;
    }
    return counter;
}

void ToolbarSelectAction::addAction(QAction *action)
{
    menu()->addAction(action);
}

void ToolbarSelectAction::addSeparator()
{
    menu()->addSeparator();
}

QAction* ToolbarSelectAction::action(int i)
{
    QList<QAction*> actionList = menu()->actions();
    if(i < 0 || i >= actionList.size()) {
        return Q_NULLPTR;
    }
    return actionList.at(i);
}

int ToolbarSelectAction::currentItem() const
{
    return m_currentItem;
}

QAction* ToolbarSelectAction::currentAction()
{
    return action(m_currentItem);
}

bool ToolbarSelectAction::containsAction(QAction *action)
{
    return actionIndex(action) >= 0;
}

void ToolbarSelectAction::setCurrentItem(int i)
{
    setCurrentAction(action(i));
}

void ToolbarSelectAction::setCurrentAction(QAction *action)
{
    if(!action) {
        return;
    }
    int index = actionIndex(action);
    if(index < 0) {
        return;
    }
    setIcon(action->icon());
    setText(action->text());
    m_currentItem = index;
}

void ToolbarSelectAction::removeAllActions()
{
    menu()->clear();
    m_currentItem = -1;
    setText(m_mainText);
    setIcon(QIcon());
}

void ToolbarSelectAction::slotTriggered(QAction* action) {

    KILE_DEBUG_MAIN << "triggered with " << action->text();

    if( currentAction() != action ) {
        setIcon(action->icon());
        setText(action->text());
        setCurrentAction(action);
    }
}

void ToolbarSelectAction::slotMainActionTriggered()
{
    QAction *action = currentAction();
    if(action) {
        action->trigger();
    }
}

void ToolbarSelectAction::slotMainButtonPressed()
{
    QAction *action = currentAction();
    if(!action) {
        emit(mainButtonWithNoActionPressed());
    }
}

QMenu* ToolbarSelectAction::menu()
{
    if(!QAction::menu()) {
        QMenu *menu = new QMenu();
        setMenu(menu);
    }

    return qobject_cast<QMenu*>(QAction::menu());
}

QWidget* ToolbarSelectAction::createWidget(QWidget *parent)
{
    QToolBar *parentToolBar = qobject_cast<QToolBar*>(parent);
    if (!parentToolBar) {
        return QWidgetAction::createWidget(parent);
    }
    QToolButton* button = new QToolButton(parent);
    button->setAutoRaise(true);
    button->setFocusPolicy(Qt::NoFocus);
    button->setPopupMode(QToolButton::MenuButtonPopup);
    button->setIconSize(parentToolBar->iconSize());
    button->setToolButtonStyle(parentToolBar->toolButtonStyle());
    connect(parent, SIGNAL(iconSizeChanged(QSize)),
            button, SLOT(setIconSize(QSize)));
    connect(parent, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
            button, SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));
    button->setDefaultAction(this);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(slotMainActionTriggered()));
    connect(button, SIGNAL(pressed()), this, SLOT(slotMainButtonPressed()));
    connect(this, SIGNAL(mainButtonWithNoActionPressed()), button, SLOT(showMenu()));
    return button;
}

void ToolbarSelectAction::saveCurrentAction()
{
    m_savedCurrentAction = currentAction();
}

void ToolbarSelectAction::restoreCurrentAction()
{
    if(!m_savedCurrentAction) {
        return;
    }
    setCurrentAction(m_savedCurrentAction);
    m_savedCurrentAction = Q_NULLPTR;
}

