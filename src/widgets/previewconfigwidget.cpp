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

#include "previewconfigwidget.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QMap>
#include <QStringList>
#include <QValidator>
#include <QVBoxLayout>

#include <KColorButton>
#include <KComboBox>
#include <QDialog>
#include <QLineEdit>
#include <KLocalizedString>
#include <KConfigGroup>

#include "kileconfig.h"
#include "kiledebug.h"

KileWidgetPreviewConfig::KileWidgetPreviewConfig(KConfig *config, KileTool::QuickPreview *preview, QWidget *parent, const char *name)
    : QWidget(parent),
      m_config(config),
      m_preview(preview)
{
    setObjectName(name);
    // Layout
    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->setMargin(0);
//TODO PORT QT5 	vbox->setSpacing(QDialog::spacingHint());
    setLayout(vbox);

    QGroupBox *groupbox = new QGroupBox(i18n("Quick Preview in a Separate Window"), this);
    groupbox->setFlat(false);
    QGridLayout *groupboxLayout = new QGridLayout();
//TODO PORT QT5 	groupboxLayout->setMargin(QDialog::marginHint());
//TODO PORT QT5 	groupboxLayout->setSpacing(QDialog::spacingHint());
    groupboxLayout->setAlignment(Qt::AlignTop);
    groupbox->setLayout(groupboxLayout);

    QLabel *label = new QLabel(i18n("Select a configuration:"), groupbox);
    label->setObjectName("label");
    m_combobox = new KComboBox(false, groupbox);
    m_combobox->setObjectName("combobox");

    groupboxLayout->addWidget(label, 0, 0);
    groupboxLayout->addWidget(m_combobox, 0, 2);
    groupboxLayout->setColumnMinimumWidth(1, 8);
    groupboxLayout->setColumnStretch(3, 1);

    QGroupBox *gbResolution = new QGroupBox(i18n("Quick Preview in Bottom Bar"), this);
    gbResolution->setFlat(false);
    QGridLayout *resLayout = new QGridLayout();
//TODO PORT QT5 	resLayout->setMargin(QDialog::marginHint());
//TODO PORT QT5 	resLayout->setSpacing(QDialog::spacingHint());
    resLayout->setAlignment(Qt::AlignTop);
    gbResolution->setLayout(resLayout);

    QLabel *resLabel = new QLabel(i18n("&Resolution:"), gbResolution);
    m_leDvipngResolution = new QLineEdit(gbResolution);
    QLabel *resDpi = new QLabel(i18n("dpi"), gbResolution);
    QLabel *resAllowed = new QLabel(i18n("(allowed values: 30-1000 dpi)"), gbResolution);

    QLabel *backgroundColorLabel = new QLabel(i18n("&Background Color:"), gbResolution);
    m_backgroundColorButton = new KColorButton(gbResolution);
    m_backgroundColorButton->setDefaultColor(QColor(Qt::white));
    backgroundColorLabel->setBuddy(m_backgroundColorButton);
    // set validator
    QValidator* validator = new QIntValidator(30, 1000, this);
    m_leDvipngResolution->setValidator(validator);
    resLabel->setBuddy(m_leDvipngResolution);

    QString sep = "&nbsp;&nbsp;&nbsp;&nbsp;";
    QString title = i18n("Kile supports three kinds of conversion to png images");
    QString tool1 = i18n("dvi --> png") + sep + i18n("(uses dvipng)");
    QString tool2 = i18n("dvi --> ps --> png") + sep + i18n("(uses dvips/convert)");
    QString tool3 = i18n("pdf --> png") + sep + i18n("(uses convert)");
    QString description = QString("%1:<ul><li>%2<li>%3<li>%4</ul>").arg(title).arg(tool1).arg(tool2).arg(tool3);

    QLabel *labelDescription = new QLabel(description, gbResolution);
    QLabel *labelDvipng = new QLabel(i18n("dvipng:"), gbResolution);
    QLabel *labelConvert = new QLabel(i18n("convert:"), gbResolution);
    m_lbDvipng = new QLabel(gbResolution);
    m_lbConvert = new QLabel(gbResolution);

    resLayout->addWidget(resLabel, 0, 0);
    resLayout->addWidget(m_leDvipngResolution, 0, 2);
    resLayout->addWidget(resDpi, 0, 3);
    resLayout->addWidget(resAllowed, 0, 5, Qt::AlignLeft);
    resLayout->addWidget(backgroundColorLabel, 1, 0);
    resLayout->addWidget(m_backgroundColorButton, 1, 2);
    resLayout->addWidget(labelDescription, 2, 0, 1, 6);
    resLayout->addWidget(labelDvipng, 3, 0);
    resLayout->addWidget(m_lbDvipng, 3, 2);
    resLayout->addWidget(labelConvert, 4, 0);
    resLayout->addWidget(m_lbConvert, 4, 2);
    resLayout->setColumnMinimumWidth(1, 8);
    resLayout->setColumnMinimumWidth(4, 24);
    resLayout->setColumnStretch(5, 1);

    m_gbPreview = new QGroupBox(i18n("Properties"), this);
    m_gbPreview->setFlat(false);
    m_gbPreview->setObjectName("gbpreview");
    QGridLayout *previewLayout = new QGridLayout();
//TODO PORT QT5 	previewLayout->setMargin(QDialog::marginHint());
//TODO PORT QT5 	previewLayout->setSpacing(QDialog::spacingHint());
    previewLayout->setAlignment(Qt::AlignTop);
    m_gbPreview->setLayout(previewLayout);

    QLabel *labelPreviewWidget = new QLabel(i18n("Show preview in bottom bar:"), m_gbPreview);
    QLabel *labelPreviewType = new QLabel(i18n("Conversion to image:"), m_gbPreview);
    QLabel *labelSelection = new QLabel(i18n("Selection:"), m_gbPreview);
    QLabel *labelEnvironment = new QLabel(i18n("Environment:"), m_gbPreview);
    QLabel *labelMathgroup = new QLabel(i18n("Mathgroup:"), m_gbPreview);
    QLabel *labelSubdocument1 = new QLabel(i18n("Subdocument:"), m_gbPreview);
    QLabel *labelSubdocument2 = new QLabel(i18n("Not available, opens always in a separate window."), m_gbPreview);
    m_cbSelection = new QCheckBox(m_gbPreview);
    m_cbEnvironment = new QCheckBox(m_gbPreview);
    m_cbMathgroup = new QCheckBox(m_gbPreview);
    m_coSelection = new KComboBox(false, m_gbPreview);
    m_coEnvironment = new KComboBox(false, m_gbPreview);
    m_coMathgroup = new KComboBox(false, m_gbPreview);

    previewLayout->addWidget(labelPreviewWidget, 0, 0, 1, 3);
    previewLayout->addWidget(labelPreviewType, 0, 4);
    previewLayout->addWidget(labelSelection, 1, 0);
    previewLayout->addWidget(m_cbSelection, 1, 2);
    previewLayout->addWidget(m_coSelection, 1, 4);
    previewLayout->addWidget(labelEnvironment, 2, 0);
    previewLayout->addWidget(m_cbEnvironment, 2, 2);
    previewLayout->addWidget(m_coEnvironment, 2, 4);
    previewLayout->addWidget(labelMathgroup, 3, 0);
    previewLayout->addWidget(m_cbMathgroup, 3, 2);
    previewLayout->addWidget(m_coMathgroup, 3, 4);
    previewLayout->addWidget(labelSubdocument1, 4, 0);
    previewLayout->addWidget(labelSubdocument2, 4, 2, 1, 4, Qt::AlignLeft);
    previewLayout->setRowMinimumHeight(0, 3 * labelPreviewWidget->sizeHint().height() / 2);
    previewLayout->setRowMinimumHeight(3, m_coEnvironment->sizeHint().height());
    previewLayout->setColumnMinimumWidth(1, 12);
    previewLayout->setColumnMinimumWidth(3, 40);
    previewLayout->setColumnStretch(5, 1);

    vbox->addWidget(groupbox);
    vbox->addWidget(gbResolution);
    vbox->addWidget(m_gbPreview);
    vbox->addStretch();

    connect(m_cbEnvironment, SIGNAL(clicked()), this, SLOT(updateConversionTools()));
    connect(m_cbSelection, SIGNAL(clicked()), this, SLOT(updateConversionTools()));
    connect(m_cbMathgroup, SIGNAL(clicked()), this, SLOT(updateConversionTools()));
}

//////////////////// read/write configuration ////////////////////

void KileWidgetPreviewConfig::readConfig()
{
    setupSeparateWindow();
    setupBottomBar();
    setupProperties();

    updateConversionTools();
}

void KileWidgetPreviewConfig::writeConfig()
{
    KileConfig::setPreviewTask(m_combobox->currentText());

    bool ok;
    QString resolution = m_leDvipngResolution->text();
    int dpi = resolution.toInt(&ok);
    if(ok) {
        if(dpi < 30) {
            resolution = "30";
        }
        else {
            if(dpi > 1000) {
                resolution = "1000";
            }
        }
        KileConfig::setDvipngResolution(resolution);
    }
    KileConfig::setPreviewPaneBackgroundColor(m_backgroundColorButton->color());
    if(m_gbPreview->isEnabled()) {
        KileConfig::setSelPreviewInWidget(m_cbSelection->isChecked());
        KileConfig::setEnvPreviewInWidget(m_cbEnvironment->isChecked());
        KileConfig::setMathgroupPreviewInWidget(m_cbMathgroup->isChecked());
    }
    else {
        KileConfig::setEnvPreviewInWidget(false);
        KileConfig::setSelPreviewInWidget(false);
        KileConfig::setMathgroupPreviewInWidget(false);
    }

    KileConfig::setSelPreviewTool(index2tool(m_coSelection->currentIndex()));
    KileConfig::setEnvPreviewTool(index2tool(m_coEnvironment->currentIndex()));
    KileConfig::setMathgroupPreviewTool(index2tool(m_coMathgroup->currentIndex()));
}

void KileWidgetPreviewConfig::setupSeparateWindow()
{
    // get all possible tasks for QuickPreview in a separate window
    QStringList tasklist;
    m_preview->getTaskList(tasklist);

    // split them into group and combobox entry
    m_combobox->clear();
    for(int i = 0; i < tasklist.count(); ++i) {
        QStringList list = tasklist[i].split('=');
        if (m_config->hasGroup(list[0])) {
            m_combobox->addItem(list[1]);
        }
    }

    // set current task
    m_combobox->setCurrentIndex(m_combobox->findText(KileConfig::previewTask()));
}

void KileWidgetPreviewConfig::setupBottomBar()
{
    // setup resolution for QuickPreview in bottom bar
    m_leDvipngResolution->setText(KileConfig::dvipngResolution());

    // setup tools
    m_dvipngInstalled = KileConfig::dvipng();
    m_convertInstalled = KileConfig::convert();

    m_backgroundColorButton->setColor(KileConfig::previewPaneBackgroundColor());

    m_lbDvipng->setText((m_dvipngInstalled) ? i18n("installed") : i18n("not installed"));
    m_lbConvert->setText((m_convertInstalled) ? i18n("installed") : i18n("not installed"));
}

void KileWidgetPreviewConfig::setupProperties()
{
    // setup properties for QuickPreview
    m_cbSelection->setChecked(KileConfig::selPreviewInWidget());
    m_cbEnvironment->setChecked(KileConfig::envPreviewInWidget());
    m_cbMathgroup->setChecked(KileConfig::mathgroupPreviewInWidget());

    // setup conversion tools
    QStringList toollist;
    if(m_dvipngInstalled) {
        toollist << i18n("dvi --> png");
    }
    if(m_convertInstalled) {
        toollist << i18n("dvi --> ps --> png");
        toollist << i18n("pdf --> png");
    }

    // setup comboboxes
    if(installedTools() == 0) {
        m_gbPreview->setEnabled(false);
    }
    else {
        m_coSelection->addItems(toollist);
        m_coEnvironment->addItems(toollist);
        m_coMathgroup->addItems(toollist);

        m_coSelection->setCurrentIndex(tool2index(KileConfig::selPreviewTool()));
        m_coEnvironment->setCurrentIndex(tool2index(KileConfig::envPreviewTool()));
        m_coMathgroup->setCurrentIndex(tool2index(KileConfig::mathgroupPreviewTool()));
    }
}

//////////////////// manage tools ////////////////////

// Tool is 0 (dvi->png), 1 (dvi->ps->png) or 2 (pdf->png).
// But this may not be valid, when tools are not installed anymore.
// So we have to calc a new index for the combobox
//  available = 0:   doesn't matter, everything will be disabled
//  available = 1:   only dvipng, so we use index 0
//  available = 2:   only convert: 0->0, 1->0, 2->1
//  available = 3:   dvipng/convert: index is valid

int KileWidgetPreviewConfig::tool2index(int tool)
{
    int result = 0;

    int available = installedTools();
    if(available == 3) {
        result = tool;
    }
    else {
        if(available == 2 && tool > 0) {
            result = tool - 1;
        }
    }

    return result;
}

int KileWidgetPreviewConfig::index2tool(int index)
{
    int result = 0;

    int available = installedTools();
    if(available == 3) {
        result = index;
    }
    else {
        if(available == 2) {
            result = index + 1;
        }
    }

    return result;
}

// calc installed tools:
//  - 0 : no tools installed
//  - 1 : dvipng installed
//  - 2 : convert installed
//  - 3 : dvipng/convert installed

int KileWidgetPreviewConfig::installedTools()
{
    int tools = 0;
    if(m_dvipngInstalled) {
        tools += 1;
    }
    if(m_convertInstalled) {
        tools += 2;
    }

    return tools;
}

void KileWidgetPreviewConfig::updateConversionTools()
{
    m_coSelection->setEnabled(m_cbSelection->isChecked());
    m_coEnvironment->setEnabled(m_cbEnvironment->isChecked());
    m_coMathgroup->setEnabled(m_cbMathgroup->isChecked());
}

