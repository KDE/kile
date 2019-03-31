/***********************************************************************************************
    begin                : Sun Jul 29 2001
    copyright            : (C) 2001 - 2003 by Brachet Pascal
                               2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
 ***********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DOCPART_H
#define DOCPART_H

#include <QStringList>
#include <KHTMLPart>

class DocumentationViewer : public KHTMLPart
{
    Q_OBJECT

public:
    explicit DocumentationViewer(QWidget *parent = Q_NULLPTR);
    ~DocumentationViewer();

    bool backEnable();
    bool forwardEnable();

public Q_SLOTS:
    void home();
    void forward();
    void back();
    void addToHistory(const QString& url);

Q_SIGNALS:
    void updateStatus(bool back, bool forward);

protected:
    virtual bool urlSelected(const QString &url,
                             int button,
                             int state,
                             const QString &_target,
                             const KParts::OpenUrlArguments &args = KParts::OpenUrlArguments(),
                             const KParts::BrowserArguments &browserArgs = KParts::BrowserArguments()) override;

private:
    QStringList	m_history;
    int		m_hpos;
};

#endif
