/********************************************************************************
 * Copyright (C) 2005-2006 by Holger Danielsson (holger.danielsson@t-online.de) *
 *           (C) 2011 by Michel Ludwig (michel.ludwig@kdemail.net)              *
 ********************************************************************************/

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

#include <QWidget>
#include "quickpreview.h"

class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;

class KColorButton;
class KComboBox;
class KConfig;

/**
  *@author Holger Danielsson
  */

class KileWidgetPreviewConfig : public QWidget
{
    Q_OBJECT
public:
    KileWidgetPreviewConfig(KConfig *config, KileTool::QuickPreview *preview, QWidget *parent = 0, const char *name = 0);
    ~KileWidgetPreviewConfig() {}

    void readConfig(void);
    void writeConfig(void);

private:
    KConfig *m_config;
    KComboBox *m_combobox;
    KileTool::QuickPreview *m_preview;
    QLineEdit *m_leDvipngResolution;
    QLabel *m_lbDvipng, *m_lbConvert;
    QCheckBox *m_cbEnvironment, *m_cbSelection, *m_cbMathgroup;
    KComboBox *m_coSelection, *m_coEnvironment, *m_coMathgroup;
    QGroupBox *m_gbPreview;
    KColorButton *m_backgroundColorButton;

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
