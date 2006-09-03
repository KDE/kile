/***************************************************************************
    date                 : Aug 22 2006
    version              : 0.20
    copyright            : (C) 2005-2006 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PREVIEWCONFIGWIDGET_H
#define PREVIEWCONFIGWIDGET_H

#include "quickpreview.h"

#include <qwidget.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kconfig.h>

/**
  *@author Holger Danielsson
  */

class KileWidgetPreviewConfig : public QWidget
{
	Q_OBJECT
public: 
	KileWidgetPreviewConfig(KConfig *config, KileTool::QuickPreview *preview, QWidget *parent=0, const char *name=0);
	~KileWidgetPreviewConfig() {}

	void readConfig(void);
	void writeConfig(void);

private:
	KConfig *m_config;
	KComboBox *m_combobox;
	KileTool::QuickPreview *m_preview;
	KLineEdit *m_leDvipngResolution;
};

#endif
