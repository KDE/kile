/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void KileWidgetEncodingConfig::init()
{
    m_cbDefaultEncoding->insertStringList(KGlobal::charsets()->availableEncodingNames());
}

QString KileWidgetEncodingConfig::encoding()
{
    return m_cbDefaultEncoding->currentText();
}

void KileWidgetEncodingConfig::setEncoding(const QString &enc)
{
    m_cbDefaultEncoding->setCurrentText(enc);
}
