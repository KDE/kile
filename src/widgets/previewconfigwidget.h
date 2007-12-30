/***************************************************************************
    date                 : Sep 05 2006
    version              : 0.32
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
#include <qcheckbox.h>
#include <q3groupbox.h>
//Added by qt3to4:
#include <QLabel>
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
	QLabel *m_lbDvipng, *m_lbConvert, *m_lbMathgroup;
	QCheckBox *m_cbEnvironment, *m_cbSelection, *m_cbMathgroup;
	KComboBox *m_coSelection, *m_coEnvironment;
	Q3GroupBox *m_gbPreview;

	bool m_dvipngInstalled, m_convertInstalled;

	int tool2index(int tool);
	int index2tool(int index);
	int installedTools();

	void setupSeparateWindow();
	void setupBottomBar();
	void setupProperties();

private Q_SLOTS:
	void updateConversionTools();
};

#endif
