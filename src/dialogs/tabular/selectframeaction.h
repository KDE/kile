/***************************************************************************
    begin                : Sunday Jun 27 2008
    copyright            : (C) 2008 by Mathias Soeken
    email                : msoeken@informatik.uni-bremen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SELECTFRAMEACTION_H
#define SELECTFRAMEACTION_H

#include <KToolBarPopupAction>

class QToolBar;
class QToolButton;

class QPushButton;

namespace KileDialog {

class TabularFrameWidget;

class SelectFrameAction : public KToolBarPopupAction {
    Q_OBJECT

public:
    SelectFrameAction(const QString &text, QToolBar *parent);

private:
    QIcon generateIcon();

private:
    QToolButton *m_pbNone, *m_pbLeftRight, *m_pbTopBottom, *m_pbAll;
    TabularFrameWidget *m_FrameWidget;
    QPushButton *m_pbDone;
    QToolBar *m_Parent;
    int m_CurrentBorder;

private Q_SLOTS:
    void slotTriggered();
    void slotNoneClicked();
    void slotLeftRightClicked();
    void slotTopBottomClicked();
    void slotAllClicked();
    void slotDoneClicked();

Q_SIGNALS:
    void borderSelected(int border);
};

}

#endif
