/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
void KileWidgetHelpConfig::init()
{
    m_hconfig = new ::HelpConfig(kapp->config());
    m_hconfig->readConfig();
}
    
void KileWidgetHelpConfig::readConfig()
{
	m_leLocation->setText(m_hconfig->location());
	m_rbKileRef->setChecked(m_hconfig->useKileRefForContext());
	m_rbTeTeX->setChecked(!m_hconfig->useKileRefForContext());
}
    
void KileWidgetHelpConfig::writeConfig()
{
    m_hconfig->setLocation(m_leLocation->text());
    m_hconfig->setUseKileRefForContext(m_rbKileRef->isChecked());
    m_hconfig->writeConfig();
}

