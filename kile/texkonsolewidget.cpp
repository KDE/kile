/***************************************************************************
                          texkonsolewidget.cpp  -  description
                             -------------------
    begin                : Sat Dec 8 2001
    copyright            : (C) 2001 by Brachet Pascal
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "texkonsolewidget.h"

#include <klibloader.h>
#include <kurl.h>


TexKonsoleWidget::TexKonsoleWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
    KLibFactory *factory = KLibLoader::self()->factory("libkonsolepart");
    if (!factory) return;

    part = (KParts::ReadOnlyPart *) factory->create(this);
    if (!part) return;
    if (part->widget()->inherits("QFrame"))
      ((QFrame*)part->widget())->setFrameStyle(QFrame::Panel|QFrame::Sunken);

    vbox = new QVBoxLayout(this);
    vbox->addWidget(part->widget());
    present=true;
    connect ( part, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );
}


TexKonsoleWidget::~TexKonsoleWidget()
{

}


void TexKonsoleWidget::SetDirectory(QString dirname)
{
    if (present)
    {
      KURL url(dirname);
	  if (part->url() != url)
      	part->openURL(url);
    }
}


void TexKonsoleWidget::showEvent(QShowEvent *ev)
{
     QWidget::showEvent(ev);
     activate();
}

void TexKonsoleWidget::activate()
{
    if (present)
    {

      part->widget()->show();
      this->setFocusProxy(part->widget());
      part->widget()->setFocus();
    }
}

void TexKonsoleWidget::slotDestroyed ()
{
present=false;
}

#include "texkonsolewidget.moc"
