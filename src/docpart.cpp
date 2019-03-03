/*************************************************************************************************
    begin                : Sun Jul 29 2001
    copyright            : (C) 2001 - 2003 by Brachet Pascal
                               2004 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2008 - 2019 by Michel Ludwig (michel.ludwig@kdemail.net)
 *************************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "docpart.h"

#include <KConfig>
#include <KMessageBox>
#include <KMimeTypeTrader>
#include <KStandardAction>
#include <KRun>
#include <KHTMLView>

#include <QMimeDatabase>
#include <QMimeType>

#include "kiledebug.h"
#include "utilities.h"

DocumentationViewer::DocumentationViewer(QWidget *parent) : KHTMLPart(parent, parent, BrowserViewGUI)
{
    m_hpos = 0;
    QString rc = KileUtilities::locate(QStandardPaths::AppDataLocation, "docpartui.rc");
    setXMLFile(rc);
    KStandardAction::back(this, SLOT(back()), (QObject*)actionCollection());
    KStandardAction::forward(this, SLOT(forward()), (QObject*)actionCollection());
    KStandardAction::home(this, SLOT(home()), (QObject*)actionCollection());
}

DocumentationViewer::~DocumentationViewer()
{
}

bool DocumentationViewer::urlSelected(const QString &url, int button, int state, const QString &_target, const KParts::OpenUrlArguments &args, const KParts::BrowserArguments & /* browserArgs */)
{
    QUrl cURL = completeURL(url);
    QMimeDatabase db;
    QString mime = db.mimeTypeForUrl(cURL).name();

    //load this URL in the embedded viewer if KHTML can handle it, or when mimetype detection failed
    KService::Ptr service = KService::serviceByDesktopName("khtml");
    if (db.mimeTypeForUrl(cURL).isDefault() || (service && service->hasServiceType(mime))) {
        KHTMLPart::urlSelected(url, button, state, _target, args);
        openUrl(cURL);
        addToHistory(cURL.url());
    }
    //KHTML can't handle it, look for an appropriate application
    else {
        KService::List offers = KMimeTypeTrader::self()->query(mime, "Type == 'Application'");
        if(offers.isEmpty()) {
            KMessageBox::error(view(), i18n("No KDE service found for the MIME type \"%1\".", mime));
            return false;
        }
        QList<QUrl> lst;
        lst.append(cURL);
        KRun::runService(*(offers.first()), lst, view());
    }
    return true;
}

void DocumentationViewer::home()
{
    if(!m_history.isEmpty()) {
        openUrl(QUrl::fromLocalFile(m_history.first()));
    }
}

void DocumentationViewer::forward()
{
    if(forwardEnable()) {
        ++m_hpos;
        openUrl(QUrl::fromLocalFile(m_history[m_hpos]));
        emit updateStatus(backEnable(), forwardEnable());
    }
}


void DocumentationViewer::back()
{
    if(backEnable()) {
        --m_hpos;
        openUrl(QUrl::fromLocalFile(m_history[m_hpos]));
        emit updateStatus(backEnable(), forwardEnable());
    }
}


void DocumentationViewer::addToHistory(const QString& url)
{
    if(m_history.count() > 0) {
        while(m_hpos < m_history.count() - 1) {
            m_history.pop_back();
        }
    }

    if(!m_history.isEmpty()) {
        ++m_hpos;
    }

    m_history.append(url);

    m_hpos = m_history.count() - 1;
    emit updateStatus(backEnable(), forwardEnable());
}


bool DocumentationViewer::backEnable()
{
    return (m_hpos > 0);
}


bool DocumentationViewer::forwardEnable()
{
    return (m_hpos < m_history.count() - 1);
}

