/******************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 ******************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QRegExpValidator>
#include <QVariant>
#include <QMap>
#include <QFile>
#include <QFileInfo>

#include <KMessageBox>
#include <KFileDialog>
#include <KInputDialog>
#include <KTextEditor/View>

#include "scripting/kilescriptobject.h"
#include "kileviewmanager.h"
#include "kileinfo.h"

namespace KileScript {

////////////////////////////////// KileAlert object //////////////////////////////////////

KileAlert::KileAlert(QObject *parent, KParts::MainWindow *mainWindow)
   : QObject(parent), m_mainWindow(mainWindow)
{
}

void KileAlert::information(const QString &text, const QString &caption)
{
	QString msgCaption = ( caption.isEmpty() ) ? i18n("Script: information") : caption;
	KMessageBox::information(m_mainWindow,text,msgCaption);
}

void KileAlert::sorry(const QString &text, const QString &caption)
{
	QString msgCaption = ( caption.isEmpty() ) ? i18n("Script: sorry") : caption;
	KMessageBox::sorry(m_mainWindow,text,msgCaption);
}

void KileAlert::error(const QString &text, const QString &caption)
{
	QString msgCaption = ( caption.isEmpty() ) ? i18n("Script: error") : caption;
	KMessageBox::error(m_mainWindow,text,msgCaption);
}

QString KileAlert::question(const QString &text, const QString &caption)
{
	QString msgCaption = ( caption.isEmpty() ) ? i18n("Script: question") : caption;
	return ( KMessageBox::questionYesNo(m_mainWindow,text,msgCaption) == KMessageBox::No ) ? "no" : "yes";
}

QString KileAlert::warning(const QString &text, const QString &caption)
{
	QString msgCaption = ( caption.isEmpty() ) ? i18n("Script: warning") : caption;
	return ( KMessageBox::warningContinueCancel(m_mainWindow,text,msgCaption) == KMessageBox::Continue ) ? "continue" : "cancel";
}

////////////////////////////////// KileInput object //////////////////////////////////////

KileInput::KileInput(QObject *parent)
   : QObject(parent)
{
}

QString KileInput::getListboxItem(const QString &caption, const QString &label, const QStringList &list)
{
	return getItem(caption,label,list,false);
}

QString KileInput::getComboboxItem(const QString &caption, const QString &label, const QStringList &list)
{
	return getItem(caption,label,list,true);
}

QString KileInput::getText(const QString &caption, const QString &label)
{
	QStringList list = checkCaptionAndLabel(caption, label);
	return KInputDialog::getText(list[0], list[1], QString(), NULL, NULL);
}

QString KileInput::getLatexCommand(const QString &caption, const QString &label)
{
	QRegExpValidator validator(QRegExp("[A-Za-z]+"),this);
	QStringList list = checkCaptionAndLabel(caption, label);
	return KInputDialog::getText(list[0], list[1], QString(), NULL, NULL, &validator);
}

int KileInput::getInteger(const QString &caption, const QString &label, int min, int max)
{
	QStringList list = checkCaptionAndLabel(caption, label);
	return KInputDialog::getInteger(list[0], list[1], 0, min, max, 1, 10, NULL, NULL);
}

int KileInput::getPosInteger(const QString &caption, const QString &label, int min, int max)
{
	return getInteger(caption,label,min,max);
}

QString KileInput::getItem(const QString &caption, const QString &label, const QStringList &itemlist, bool combobox)
{
	QStringList list = checkCaptionAndLabel(caption, label);
	return KInputDialog::getItem(list[0], list[1], itemlist, 0, combobox, NULL, NULL);
}

QStringList KileInput::checkCaptionAndLabel(const QString &caption, const QString &label)
{
	QString checkedCaption = caption, checkedLabel = label;
	if(caption.isEmpty()) {
		checkedCaption = i18n("Enter Value");
	}
	if(label.isEmpty()) {
		checkedLabel = i18n("Please enter a value");
	}

	return QStringList() << checkedCaption << checkedLabel;
}

////////////////////////////////// KileWizard object //////////////////////////////////////

KileWizard::KileWizard(QObject *parent, KileInfo *kileInfo, const QMap<QString,QAction *> *scriptActions)
   : QObject(parent), m_kileInfo(kileInfo), m_scriptActions(scriptActions)
{
}

void KileWizard::tabular()
{
	triggerAction("wizard_tabular");
}

void KileWizard::array()
{
	triggerAction("wizard_array");
}

void KileWizard::tabbing()
{
	triggerAction("wizard_tabbing");
}

void KileWizard::floatEnvironment()
{
	triggerAction("wizard_float");
}

void KileWizard::mathEnvironment()
{
	triggerAction("wizard_mathenv");
}

void KileWizard::postscript()
{
	triggerAction("wizard_postscript");
}

void KileWizard::pdf()
{
	triggerAction("wizard_pdf");
}

bool KileWizard::triggerAction(const QString &name)
{
	if ( name=="wizard_postscript" || name=="wizard_pdf" ) {
		KTextEditor::View *view = m_kileInfo->viewManager()->currentTextView();
		if ( !view ) {
			return false;
		}
	}

	if ( m_scriptActions->contains(name) ) {
		m_scriptActions->value(name)->trigger();
		return true;
	}
	else {
		return false;
	}
}

////////////////////////////////// KileJavaScript object //////////////////////////////////////

KileJavaScript::KileJavaScript(QObject *parent)
   : QObject(parent)
{
}

QString KileJavaScript::caption()
{
	return i18n("Script '%1.js'", m_scriptname);
}

////////////////////////////////// KileFile object //////////////////////////////////////

KileFile::KileFile(QObject *parent, KileInfo *kileInfo)
   : QObject(parent),  m_kileInfo(kileInfo)
{
}

QMap<QString, QVariant> KileFile::read(const QString& filename) const
{
	QMap<QString,QVariant> result;
//	result["status"] = QVariant();
	result["message"] = QString();
	result["text"] = QString();

	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		result["message"] = i18n("File Handling Error: Unable to find the file '%1'", filename);
		result["status"] = KileFile::ACCESS_FAILED;
		return result;
	}

	// read data
	QTextStream stream(&file);
	stream.setCodec("UTF-8");
	result["text"] = stream.readAll();
	file.close();

	result["status"] = KileFile::ACCESS_OK;

	return result;
}

QMap<QString, QVariant> KileFile::read() const
{
	QString openedFile = m_kileInfo->getName();
	const QString filepath = (!openedFile.isEmpty()) ? QFileInfo(m_kileInfo->getName()).absolutePath() : QString();
	QString filename = KFileDialog::getOpenFileName(filepath, QString(), m_kileInfo->mainWindow(), i18n("Select File to Read"));
	if(!filename.isEmpty()) {
		return read(filename);
	}
	else {
		return actionCancelled();
	}
}

QMap<QString, QVariant> KileFile::write(const QString& filename, const QString& text) const
{
	QMap<QString, QVariant> result;

	QFile file(filename);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		result["status"] = KileFile::ACCESS_FAILED;
		result["message"] = i18n("File Handling Error: Unable to create the output file '%1'", filename);
		return result;
	}

	// write data
	qint64 bytesWritten = 0;
	const QByteArray dataToWrite = text.toUtf8();
	while(bytesWritten < dataToWrite.size()) {
		qint64 bytesWrittenNow = file.write(dataToWrite.constData() + bytesWritten, dataToWrite.size() - bytesWritten);
		if(bytesWrittenNow < 0) {
			result["status"] = KileFile::ACCESS_FAILED;
			result["message"] = i18n("File Handling Error: Unable to write to the output file '%1'", filename);
			file.close();
			return result;
		}
		bytesWritten += bytesWrittenNow;
	}
	file.close();

	result["status"] = KileFile::ACCESS_OK;
	result["message"] = QString();

	return result;
}

QMap<QString, QVariant> KileFile::write(const QString& text) const
{
	QString openedFile = m_kileInfo->getName();
	const QString filepath = (!openedFile.isEmpty()) ? QFileInfo(m_kileInfo->getName()).absolutePath() : QString();
	QString filename = KFileDialog::getSaveFileName(filepath, QString(), m_kileInfo->mainWindow(), i18n("Save As"));
	if(!filename.isEmpty()) {
		return write(filename, text);
	}
	else {
		return actionCancelled();
	}
}

QString KileFile::getOpenFileName(const KUrl& url, const QString& filter)
{
	KUrl startdir = (url.isEmpty()) ? KUrl(QFileInfo(m_kileInfo->getName()).absolutePath()) : url;
	return KFileDialog::getOpenFileName(startdir, filter, m_kileInfo->mainWindow(), i18n("Select File to Read"));
}

QString KileFile::getSaveFileName(const KUrl& url, const QString& filter)
{
	KUrl startdir = (url.isEmpty()) ? KUrl(QFileInfo(m_kileInfo->getName()).absolutePath()) : url;
	return KFileDialog::getSaveFileName(startdir, filter, m_kileInfo->mainWindow(), i18n("Save As"));
}

QMap<QString,QVariant> KileFile::actionCancelled() const
{
	QMap<QString,QVariant> result;
	result["status"] = KileFile::ACCESS_FAILED;
	result["message"] = i18n("This action was canceled by the user.");
	result["text"] = QString();
	return result;
}

////////////////////////////////// KileScript object //////////////////////////////////////

KileScriptObject::KileScriptObject(QObject *parent, KileInfo* kileInfo, const QMap<QString,QAction *> *scriptActions)
   : QObject(parent), m_kileInfo(kileInfo)
{
	m_kileAlert = new KileAlert(this,m_kileInfo->mainWindow());
	m_kileInput = new KileInput(this);
	m_kileWizard = new KileWizard(this,m_kileInfo,scriptActions);
	m_kileScript = new KileJavaScript(this);
	m_kileFile = new KileFile(this,m_kileInfo);
}

void KileScriptObject::setScriptname(const QString &name)
{
	m_kileScript->setScriptname(name);

}


}

#include "kilescriptobject.moc"
