/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void KileWidgetLatexConfig::readConfig()
{
    if(NULL != m_lconfig) {
	m_chComplete->setChecked(m_lconfig->completeEnvironment());	   m_leResolution->setText(m_lconfig->resolution());
	m_chBBox->setChecked(m_lconfig->determineBoundingBox());
    
	QString installed;
	if(m_lconfig->hasImageMagick())
	    installed = i18n("installed");
	else
	    installed = i18n("not installed");
	m_tlImageMagickInst->setText(installed);
    }
}


void KileWidgetLatexConfig::writeConfig()
{
    if(NULL != m_lconfig) {
	m_lconfig->completeEnvironment(m_chComplete->isChecked());
	m_lconfig->resolution(m_leResolution->text());
	m_lconfig->determineBoundingBox(m_chBBox->isChecked());
	m_lconfig->writeConfig();
    }
}


void KileWidgetLatexConfig::init()
{
    m_lconfig = new ::LatexConfig(kapp->config());
    if(NULL != m_lconfig)
	m_lconfig->readConfig();
}
