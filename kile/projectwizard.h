/***************************************************************************
                          projectwizard.h  -  description
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

#ifndef PROJECTWIZARD_H
#define PROJECTWIZARD_H

#include <kdialogbase.h>

/**
  *@author Jeroen Wijnhout
  */

class ProjectWizard : public KDialogBase  {
   Q_OBJECT
public: 
	ProjectWizard(QWidget *parent=0, const char *name=0);
	~ProjectWizard();

};

#endif
