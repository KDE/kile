/********************************************************************************
*   Copyright (C) 2018 by Michel Ludwig (michel.ludwig@kdemail.net)             *
*                 2009 by Holger Danielsson (holger.danielsson@versanet.de)     *
*********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef COMMANDVIEW_H
#define COMMANDVIEW_H

#include <QListWidget>
#include <QToolBox>

#include "kileinfo.h"
#include "codecompletion.h"

class QComboBox;

namespace KileWidget {

class CommandView : public QListWidget
{
    Q_OBJECT

public:
    explicit CommandView(QWidget *parent);
    ~CommandView();
};

class CommandViewToolBox : public QWidget
{
    Q_OBJECT

public:
    explicit CommandViewToolBox(KileInfo *ki, QWidget *parent);
    ~CommandViewToolBox();

    void readCommandViewFiles();

Q_SIGNALS:
    void sendText(const QString &text);

private Q_SLOTS:
    void slotItemActivated(QListWidgetItem *item);

private:
    KileInfo *m_ki;
    KileCodeCompletion::LaTeXCompletionModel *m_latexCompletionModel;
    QComboBox *m_cwlFilesComboBox;
    CommandView *m_commandView;

    void clearItems();
    void populateCommands(const QString& cwlFile);
};

}

#endif
