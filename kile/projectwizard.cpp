/***************************************************************************
                          projectwizard.cpp  -  description
                             -------------------
    begin                : Sat Apr 26 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : wijnhout@science.uva.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qlabel.h>
#include <qdir.h>
#include <klocale.h>

#include <kmessagebox.h>
#include "projectwizard.h"

ProjectWizard::ProjectWizard(QWidget *parent, const char *name )
  : KDialogBase(parent,name,true,i18n("New File"),KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true)
{

    
}

ProjectWizard::~ProjectWizard(){
}

#include "projectwizard.moc"
