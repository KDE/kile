/************************************************************************************************
    date                 : Mar 30 2007
    version              : 0.24
    copyright            : (C) 2004-2007 by Holger Danielsson (holger.danielsson@versanet.de)
 ************************************************************************************************/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "widgets/codecompletionconfigwidget.h"

#include <QCheckBox>
#include <QDir>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QSpinBox>
#include <QStringList>
#include <QTabWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <KConfig>
#include <KDialog>
#include <KLocale>
#include <KMessageBox>
#include <KPushButton>
#include <KStandardDirs>
#include <KTabWidget>

#include "kilelistselector.h"
#include "kileconfig.h"
#include "widgets/logwidget.h"
#include "kiletool_enums.h"

CodeCompletionConfigWidget::CodeCompletionConfigWidget(KConfig *config, KileWidget::LogWidget *logwidget, QWidget *parent, const char *name)
		: QWidget(parent), m_config(config), m_logwidget(logwidget)
{
	setObjectName(name);
	// Layout
	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setMargin(0);
	vbox->setSpacing(KDialog::spacingHint());
	setLayout(vbox);

	// Groupbox with TabDialog and two button
	QGroupBox *gb_tab = new QGroupBox(i18n("Complete Modes"), this);
	QGridLayout *grid_tab = new QGridLayout();
	grid_tab->setMargin(KDialog::marginHint());
	grid_tab->setSpacing(KDialog::spacingHint());
	gb_tab->setLayout(grid_tab);

	// create TabWidget
	tab = new KTabWidget(gb_tab);

	// add three pages: Tex/Latex, Dictionary, Abbreviation
	addPage(tab, TexPage, i18n("TeX/LaTeX"), "tex");
	addPage(tab, DictionaryPage, i18n("Dictionary"), "dictionary");
	addPage(tab, AbbreviationPage, i18n("Abbreviation"), "abbreviation");

	// add two centered button
	add = new KPushButton(i18n("Add..."), gb_tab);
	remove = new KPushButton(i18n("Remove"), gb_tab);

	grid_tab->addWidget(tab, 1, 0, 1, 2);
	grid_tab->addWidget(add, 2, 0, Qt::AlignRight);
	grid_tab->addWidget(remove, 2, 1, Qt::AlignLeft);

	// below: OptionBox
	QGroupBox *bg_options = new QGroupBox(this);
	QGridLayout *bg_optionsLayout = new QGridLayout();
	bg_optionsLayout->setAlignment(Qt::AlignTop);
	bg_optionsLayout->setMargin(KDialog::marginHint());
	bg_optionsLayout->setSpacing(KDialog::spacingHint());
	bg_options->setLayout(bg_optionsLayout);

	cb_setcursor = new QCheckBox(i18n("Place cursor"), bg_options);
	cb_setbullets = new QCheckBox(i18n("Insert bullets"), bg_options);
	cb_closeenv = new QCheckBox(i18n("Close environments"), bg_options);
	cb_usecomplete = new QCheckBox(i18n("Use complete"), bg_options);
	cb_autocomplete = new QCheckBox(i18n("Auto completion (LaTeX)"), bg_options);
	lb_latexthreshold = new QLabel(i18n("Threshold:"), bg_options);
	sp_latexthreshold = new QSpinBox(bg_options);
	sp_latexthreshold->setMinimum(1);
	sp_latexthreshold->setMaximum(9);
	sp_latexthreshold->setSingleStep(1);
	QLabel *lb_latexletters = new QLabel(i18n("letters"), bg_options);
	cb_autocompletetext = new QCheckBox(i18n("Auto completion (text)"), bg_options);
	lb_textthreshold = new QLabel(i18n("Threshold:"), bg_options);
	sp_textthreshold = new QSpinBox(bg_options);
	sp_textthreshold->setMinimum(1);
	sp_textthreshold->setMaximum(9);
	sp_textthreshold->setSingleStep(1);
	QLabel *lb_textletters = new QLabel(i18n("letters"), bg_options);
	cb_showabbrevview = new QCheckBox(i18n("Show abbreviations"), bg_options);
	cb_autocompleteabbrev = new QCheckBox(i18n("Auto completion (abbrev.)"), bg_options);
	cb_citeoutofbraces = new QCheckBox(i18n("Move out of braces (citation keylists)"), bg_options);

	bg_optionsLayout->addWidget(cb_setcursor, 0, 0);
	bg_optionsLayout->addWidget(cb_setbullets, 1, 0);
	bg_optionsLayout->addWidget(cb_closeenv, 2, 0);
	bg_optionsLayout->addWidget(cb_showabbrevview, 3, 0);
	bg_optionsLayout->addWidget(cb_usecomplete, 0, 2);
	bg_optionsLayout->addWidget(cb_autocomplete, 1, 2);
	bg_optionsLayout->addWidget(lb_latexthreshold, 1, 4);
	bg_optionsLayout->addWidget(sp_latexthreshold, 1, 6);
	bg_optionsLayout->addWidget(lb_latexletters, 1, 7);
	bg_optionsLayout->addWidget(cb_autocompletetext, 2, 2);
	bg_optionsLayout->addWidget(lb_textthreshold, 2, 4);
	bg_optionsLayout->addWidget(sp_textthreshold, 2, 6);
	bg_optionsLayout->addWidget(lb_textletters, 2, 7);
	bg_optionsLayout->addWidget(cb_autocompleteabbrev, 3, 2);
	bg_optionsLayout->addWidget(cb_citeoutofbraces, 4, 0, 1, 7);

	// tune layout
	bg_optionsLayout->setColumnMinimumWidth(1, 20);
	bg_optionsLayout->setColumnMinimumWidth(3, 12);
	bg_optionsLayout->setColumnMinimumWidth(5, 8);
	bg_optionsLayout->setColumnStretch(7, 1);

	cb_setcursor->setWhatsThis(i18n("Try to place the cursor."));
	cb_setbullets->setWhatsThis(i18n("Insert bullets, where the user must input data."));
	cb_closeenv->setWhatsThis(i18n("Also close an environment, when an opening command is inserted."));
	cb_usecomplete->setWhatsThis(i18n("Enable components of word completion."));
	cb_autocomplete->setWhatsThis(i18n("Directional or popup-based completion with TeX/LaTeX commands, which are given in all selected word completion lists. This mode can only be selected, if no other plugin for autocompletion is active."));
	cb_autocompletetext->setWhatsThis(i18n("Directional or popup-based completion from words in the current document. This mode can only be selected, if no other plugin for autocompletion is active."));
	sp_latexthreshold->setWhatsThis(i18n("Automatically show a completion list of TeX/LaTeX commands, when the word has this length."));
	sp_textthreshold->setWhatsThis(i18n("Automatically show a completion list, when the word has this length."));
	cb_citeoutofbraces->setWhatsThis(i18n("Move cursor out of braces after selecting from a citation keylist."));

	// bottom: warning
	QLabel *lb_automodes = new QLabel(i18n("Warning: all autocompletion modes will be disabled, if you enable KTextEditor plugin word completion."), this);

	// add OptionBox and TabDialog into the layout
	vbox->addWidget(gb_tab);
	vbox->addWidget(bg_options);
	vbox->addWidget(lb_automodes);
	vbox->addStretch();

	connect(tab, SIGNAL(currentChanged(QWidget*)), this, SLOT(showPage(QWidget*)));
	connect(add, SIGNAL(clicked()), this, SLOT(addClicked()));
	connect(remove, SIGNAL(clicked()), this, SLOT(removeClicked()));

	// justify height
	QTreeWidgetItem *item = new QTreeWidgetItem(m_listview[AbbreviationPage], QStringList(I18N_NOOP("Test")));
	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	int h = 6 * (m_listview[AbbreviationPage]->sizeHintForRow(0) + 1) + 1;
	for (uint i = TexPage; i < NumPages; ++i)
		m_listview[i]->setFixedHeight(h);
	delete item;

	// find resource directories for cwl files
	getCwlDirs();
}

CodeCompletionConfigWidget::~CodeCompletionConfigWidget()
{
}

void CodeCompletionConfigWidget::addPage(QTabWidget *tab, CompletionPage page, const QString &title, const QString &dirname)
{
	m_page[page] = new QWidget(tab);

	m_listview[page] = new QTreeWidget(m_page[page]);
	m_listview[page]->setHeaderLabels(QStringList() << i18n("Complete Files")
																		<< i18n("Local File"));
	m_listview[page]->setAllColumnsShowFocus(true);
	m_listview[page]->setRootIsDecorated(false);
	m_listview[page]->setSelectionMode(QAbstractItemView::ExtendedSelection);

	QGridLayout *grid = new QGridLayout();
	grid->setMargin(0);
	grid->setSpacing(KDialog::spacingHint());
	m_page[page]->setLayout(grid);
	grid->addWidget(m_listview[page], 0, 0);

	// add Tab
	tab->addTab(m_page[page], title);

	// remember directory name
	m_dirname << dirname;

	connect(m_listview[page], SIGNAL(itemSelectionChanged()),
					this, SLOT(slotSelectionChanged()));
}

//////////////////// read/write configuration ////////////////////

void CodeCompletionConfigWidget::readConfig(void)
{
	// read selected and deselected filenames with wordlists
	m_wordlist[TexPage] = KileConfig::completeTex();
	m_wordlist[DictionaryPage]  = KileConfig::completeDict();
	m_wordlist[AbbreviationPage]  = KileConfig::completeAbbrev();

	// set checkbox status
	cb_usecomplete->setChecked(KileConfig::completeEnabled());
	cb_setcursor->setChecked(KileConfig::completeCursor());
	cb_setbullets->setChecked(KileConfig::completeBullets());
	cb_closeenv->setChecked(KileConfig::completeCloseEnv());
	cb_showabbrevview->setChecked(KileConfig::completeShowAbbrev());
	cb_citeoutofbraces->setChecked(KileConfig::completeCitationMove());

	// set checkboxes and thresholds for autocompletion modes
	if (kateCompletionPlugin())
	{
		cb_autocomplete->setChecked(false);
		cb_autocompletetext->setChecked(false);
		cb_autocompleteabbrev->setChecked(false);
	}
	else
	{
		cb_autocomplete->setChecked(KileConfig::completeAuto());
		cb_autocompletetext->setChecked(KileConfig::completeAutoText());
		cb_autocompleteabbrev->setChecked(KileConfig::completeAutoAbbrev());
	}
	sp_latexthreshold->setValue(KileConfig::completeAutoThreshold());
	sp_textthreshold->setValue(KileConfig::completeAutoTextThreshold());

	// insert filenames into listview
	for (uint i = TexPage; i < NumPages; ++i)
		setListviewEntries(CompletionPage(i));
}

void CodeCompletionConfigWidget::writeConfig(void)
{
	// default: no changes in configuration
	bool changed = false;

	// get listview entries
	for (uint i = TexPage; i < NumPages; ++i)
		changed |= getListviewEntries(CompletionPage(i));

	// Konfigurationslisten abspeichern
	KileConfig::setCompleteTex(m_wordlist[TexPage]);
	KileConfig::setCompleteDict(m_wordlist[DictionaryPage]);
	KileConfig::setCompleteAbbrev(m_wordlist[AbbreviationPage]);

	// save checkbox status
	KileConfig::setCompleteEnabled(cb_usecomplete->isChecked());
	KileConfig::setCompleteCursor(cb_setcursor->isChecked());
	KileConfig::setCompleteBullets(cb_setbullets->isChecked());
	KileConfig::setCompleteCloseEnv(cb_closeenv->isChecked());
	KileConfig::setCompleteShowAbbrev(cb_showabbrevview->isChecked());
	KileConfig::setCompleteCitationMove(cb_citeoutofbraces->isChecked());

	// read autocompletion settings
	bool autoModeLatex = cb_autocomplete->isChecked();
	bool autoModeText = cb_autocompletetext->isChecked();
	bool autoModeAbbrev = cb_autocompleteabbrev->isChecked();
	if (kateCompletionPlugin())
	{
		if (autoModeLatex || autoModeText  || autoModeAbbrev)
		{
			QString msg = i18n("You enabled the KTextEditor-Plugin for word completion, "
												 "but this conflicts with the auto completion modes of Kile. "
												 "As only one of these completion modes can be used, the "
												 "autocompletion modes of Kile will be disabled.");
			KMessageBox::information(0L, "<center>" + msg + "</center>", i18n("Autocomplete warning"));

			// disable Kile autocomplete modes
			autoModeLatex = false;
			autoModeText = false;
			autoModeAbbrev = false;
		}
	}

	// save settings for Kile autocompletion modes
	KileConfig::setCompleteAuto(autoModeLatex);
	KileConfig::setCompleteAutoText(autoModeText);
	KileConfig::setCompleteAutoAbbrev(autoModeAbbrev);
	KileConfig::setCompleteAutoThreshold(sp_latexthreshold->value());
	KileConfig::setCompleteAutoTextThreshold(sp_textthreshold->value());

	// save changed wordlists?
	KileConfig::setCompleteChangedLists(changed);
}

// read kate plugin configuration
bool CodeCompletionConfigWidget::kateCompletionPlugin()
{
	return m_config->group("Kate Document Defaults").readEntry("KTextEditor Plugin ktexteditor_docwordcompletion", false);
}

//////////////////// listview ////////////////////

// ListView fr den Konfigurationsdialog einstellen

void CodeCompletionConfigWidget::setListviewEntries(CompletionPage page)
{
	QString listname = m_dirname[page];
	QString localdir = m_localCwlDir + listname + '/';
	QString globaldir = m_globalCwlDir + listname + '/';

	// Daten aus der Konfigurationsliste in das ListView-Widget eintragen
	m_listview[page]->setUpdatesEnabled(false);
	m_listview[page]->clear();
	QStringList::ConstIterator it;
	for (it = m_wordlist[page].begin(); it != m_wordlist[page].end(); ++it)
	{
		QString basename = (*it).right((*it).length() - 2);
		bool localExists = QFileInfo(localdir + basename + ".cwl").exists();

		QTreeWidgetItem *item = new QTreeWidgetItem(m_listview[page], QStringList(basename));
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		if (localExists)
		{
			item->setCheckState(0, (*it).at(0) == '1' ? Qt::Checked : Qt::Unchecked);
			item->setText(1, "+");
		}
		else
			if (QFileInfo(globaldir + basename + ".cwl").exists())
			{
				item->setCheckState(0, (*it).at(0) == '1' ? Qt::Checked : Qt::Unchecked);
			}
			else
			{
				item->setCheckState(0, Qt::Unchecked);
				item->setText(1, i18n("File not found"));
			}
	}

	updateColumnWidth(m_listview[page]);
	m_listview[page]->setUpdatesEnabled(true);
}

void CodeCompletionConfigWidget::updateColumnWidth(QTreeWidget *listview)
{
	listview->setColumnWidth(0, listview->columnWidth(0) + 60);
}

bool CodeCompletionConfigWidget::getListviewEntries(CompletionPage page)
{
	bool changed = false;

	// count number of entries
	int n = m_listview[page]->topLevelItemCount();

	// there are changes if this number has changed
	if(n != m_wordlist[page].count()) {
		changed = true;
	}

	// clear all stringlist with files, if there are no entries
	if (n == 0) {
		m_wordlist[page].clear();
		return changed;
	}

	// now check all entries if they have changed
	QStringList newfiles;
	int index = 0;
	QTreeWidgetItemIterator it(m_listview[page]);
	while (*it) {
		QString s = ((*it)->checkState(0) == Qt::Checked) ? "1-" : "0-";
		s += (*it)->text(0);
		newfiles.append(s);

		// check for a change
		if (m_wordlist[page][index] != s)
			changed = true;

		// go on
		++it;
	}

	// only update if there are changes
	if (changed) {
		m_wordlist[page] = newfiles;
	}

	return changed;
}

bool CodeCompletionConfigWidget::isListviewEntry(QTreeWidget *listview, const QString &filename)
{
	return listview->findItems(filename, Qt::MatchExactly).count() > 0;
}

//////////////////// tabpages parameter ////////////////////

QTreeWidget *CodeCompletionConfigWidget::getListview(QWidget *page)
{
	for (uint i = TexPage; i < NumPages; ++i)
	{
		if (page == m_page[i])
			return m_listview[i];
	}
	return 0;
}

QString CodeCompletionConfigWidget::getListname(QWidget *page)
{
	for (uint i = TexPage; i < NumPages; ++i) {
		if(page == m_page[i]) {
			return m_dirname[i];
		}
	}
	return QString();
}

//////////////////// shwo tabpages ////////////////////

void CodeCompletionConfigWidget::showPage(QWidget *page)
{
	QTreeWidget *listview = getListview(page);
	if(listview) {
		remove->setEnabled(listview->selectedItems().count() > 0);
	}
}

//////////////////// add/remove new wordlists ////////////////////

// find local and global resource directories

void CodeCompletionConfigWidget::getCwlDirs()
{
	m_localCwlDir = KStandardDirs::locateLocal("appdata", "complete/");
	m_globalCwlDir.clear();

	QStringList dirs = KGlobal::dirs()->findDirs("appdata", "complete/");
	for(QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it) {
		if((*it) != m_localCwlDir) {
			m_globalCwlDir = (*it);
			break;
		}
	}
}

// find local and global cwl files: global files are not added,
// if there is already a local file with this name. We fill a map
// with filename as key and filepath as value. Additionally all
// filenames are added to a stringlist.

void CodeCompletionConfigWidget::getCwlFiles(QMap<QString, QString> &map, QStringList &list, const QString &dir)
{
	QStringList files = QDir(dir, "*.cwl").entryList();
	for (QStringList::ConstIterator it = files.begin(); it != files.end(); ++it)
	{
		QString filename = QFileInfo(*it).fileName();
		if (! map.contains(filename))
		{
			map[filename] = dir + '/' + (*it);
			list << filename;
		}
	}
}

void CodeCompletionConfigWidget::addClicked()
{
	// determine current subdirectory for current tab page
	QString listname = getListname(tab->currentWidget());


	// get a sorted list of all cwl files from both directories
	QMap<QString, QString> filemap;
	QStringList filelist;
	getCwlFiles(filemap, filelist, m_localCwlDir + listname);
	getCwlFiles(filemap, filelist, m_globalCwlDir + listname);
	filelist.sort();

	// dialog to add cwl files
	KileListSelectorMultiple *dlg  = new KileListSelectorMultiple(filelist, i18n("Complete Files"), i18n("Select Files"), this);
	if (dlg->exec()) {
		if (dlg->currentItem() >= 0) {
			QTreeWidget *listview = getListview(tab->currentWidget());     // get current page
			QStringList filenames = dlg->selected();                   // get selected files
			for (QStringList::ConstIterator it = filenames.begin(); it != filenames.end(); ++it) {
				QString filename = *it;
				// could we accept the wordlist?
				QFileInfo fi(filemap[filename]);
				if (!filename.isEmpty() && fi.exists() && fi.isReadable()) {
					QString basename = filename.left(filename.length() - 4);

					// check if this entry already exists
					if (isListviewEntry(listview, basename)) {
						m_logwidget->printMessage(KileTool::Info, i18n("Wordlist '%1' is already used.", basename), i18n("Complete"));
						continue;
					}

					// add new entry
					QTreeWidgetItem *item = new QTreeWidgetItem(listview, QStringList(basename));
					item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
					item->setCheckState(0, Qt::Checked);
					item->setSelected(true);
					if (filemap[filename].left(m_localCwlDir.length()) == m_localCwlDir) {
						item->setText(1, "+");
					}
				}
			}
			updateColumnWidth(listview);
		}
	}
	delete dlg;

}

// delete a selected entry

void CodeCompletionConfigWidget::removeClicked()
{
	QWidget *page = tab->currentWidget();
	QTreeWidget *list = getListview(page);                              // determine page

	foreach(QTreeWidgetItem *item, list->selectedItems()) {
		delete item;
	}

	showPage(page);
}

void CodeCompletionConfigWidget::slotSelectionChanged()
{
	QTreeWidget *listview = getListview(tab->currentWidget());     // get current page
	remove->setEnabled(listview->selectedItems().count() > 0);
}

#include "codecompletionconfigwidget.moc"
